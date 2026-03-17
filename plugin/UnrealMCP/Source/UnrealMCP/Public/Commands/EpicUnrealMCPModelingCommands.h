#pragma once

#include "CoreMinimal.h"
#include "Json.h"

/**
 * Handler class for Modeling/GeometryScript-related MCP commands
 * Handles procedural mesh generation and manipulation
 */
class UNREALMCP_API FEpicUnrealMCPModelingCommands
{
public:
    FEpicUnrealMCPModelingCommands();

    // Handle modeling commands
    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    // Create a high-poly subdivided plane asset
    TSharedPtr<FJsonObject> HandleCreateSubdividedPlane(const TSharedPtr<FJsonObject>& Params);
    
    // Helper to bake dynamic mesh to static mesh asset
    TSharedPtr<FJsonObject> HandleBakeProceduralMesh(const TSharedPtr<FJsonObject>& Params);
};
