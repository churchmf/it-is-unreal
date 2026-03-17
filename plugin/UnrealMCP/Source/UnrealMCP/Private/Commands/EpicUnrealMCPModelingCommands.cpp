#include "Commands/EpicUnrealMCPModelingCommands.h"
#include "Commands/EpicUnrealMCPCommonUtils.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "UDynamicMesh.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "PackageTools.h"
#include "UObject/Package.h"
#include "Engine/StaticMesh.h"

FEpicUnrealMCPModelingCommands::FEpicUnrealMCPModelingCommands()
{
}

TSharedPtr<FJsonObject> FEpicUnrealMCPModelingCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("create_subdivided_plane"))
    {
        return HandleCreateSubdividedPlane(Params);
    }
    else if (CommandType == TEXT("bake_procedural_mesh"))
    {
        return HandleBakeProceduralMesh(Params);
    }

    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown modeling command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPModelingCommands::HandleCreateSubdividedPlane(const TSharedPtr<FJsonObject>& Params)
{
    FString AssetName;
    if (!Params->TryGetStringField(TEXT("name"), AssetName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    FString DestinationPath = TEXT("/Game/Meshes/");
    Params->TryGetStringField(TEXT("path"), DestinationPath);
    if (!DestinationPath.EndsWith(TEXT("/"))) DestinationPath += TEXT("/");

    float Width = 100.0f;
    float Height = 100.0f;
    int32 WidthSteps = 100;
    int32 HeightSteps = 100;

    Params->TryGetNumberField(TEXT("width"), Width);
    Params->TryGetNumberField(TEXT("height"), Height);
    
    double WSteps, HSteps;
    if (Params->TryGetNumberField(TEXT("width_steps"), WSteps)) WidthSteps = (int32)WSteps;
    if (Params->TryGetNumberField(TEXT("height_steps"), HSteps)) HeightSteps = (int32)HSteps;

    // Create a temporary Dynamic Mesh object
    UDynamicMesh* DynamicMesh = NewObject<UDynamicMesh>();
    if (!DynamicMesh)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create DynamicMesh object"));
    }

    // Use GeometryScript to create a rectangle
    FGeometryScriptPrimitiveOptions Options;
    Options.PolygroupMode = EGeometryScriptPrimitivePolygroupMode::PerFace;
    
    FTransform Transform = FTransform::Identity;
    
    UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendRectangleXY(
        DynamicMesh, Options, Transform, Width, Height, WidthSteps, HeightSteps);

    // Prepare asset creation
    FString PackagePath = DestinationPath + AssetName;
    
    // Delete existing asset if present
    if (UEditorAssetLibrary::DoesAssetExist(PackagePath))
    {
        UEditorAssetLibrary::DeleteAsset(PackagePath);
    }

    // Create a new package and static mesh asset
    UPackage* Package = CreatePackage(*PackagePath);
    if (!Package)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create package for the static mesh asset"));
    }

    UStaticMesh* NewMesh = NewObject<UStaticMesh>(Package, *AssetName, RF_Public | RF_Standalone);
    if (!NewMesh)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create StaticMesh asset"));
    }

    // Copy Dynamic Mesh to Static Mesh LOD 0
    FGeometryScriptCopyMeshToAssetOptions BakeOptions;
    BakeOptions.bApplyNaniteSettings = true;
    BakeOptions.NewNaniteSettings.bEnabled = true;
    
    FGeometryScriptMeshWriteLOD TargetLOD;
    TargetLOD.LODIndex = 0;
    
    EGeometryScriptOutcomePins Outcome;
    UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshToStaticMesh(
        DynamicMesh, NewMesh, BakeOptions, TargetLOD, Outcome);

    if (Outcome == EGeometryScriptOutcomePins::Failure)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to bake DynamicMesh to StaticMesh asset"));
    }

    // Notify Asset Registry and mark dirty
    FAssetRegistryModule::AssetCreated(NewMesh);
    NewMesh->MarkPackageDirty();
    UEditorAssetLibrary::SaveAsset(PackagePath);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("asset_path"), NewMesh->GetPathName());
    
    return FEpicUnrealMCPCommonUtils::CreateSuccessResponse(ResultObj);
}

TSharedPtr<FJsonObject> FEpicUnrealMCPModelingCommands::HandleBakeProceduralMesh(const TSharedPtr<FJsonObject>& Params)
{
    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented yet"));
}
