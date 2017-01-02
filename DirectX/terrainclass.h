////////////////////////////////////////////////////////////////////////////////
// Filename: terrainclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TERRAINCLASS_H_
#define _TERRAINCLASS_H_
#define NOMINMAX

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <directxmath.h>
#include <fstream>
#include <stdio.h>

#include "diamondSquare.h"
#include "cameraclass.h"

using namespace DirectX;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Class name: TerrainClass
////////////////////////////////////////////////////////////////////////////////
class TerrainClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		//XMFLOAT4 color;
		XMFLOAT3 normal;
		XMFLOAT2 texture;
	};

	struct HeightMapType
	{
		float x, y, z;
		float nx, ny, nz;
	};

	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	struct VectorType
	{
		float x, y, z;
	};

public:
	TerrainClass();
	TerrainClass(const TerrainClass&);
	~TerrainClass();

	bool Initialize(ID3D11Device * device);

	void Shutdown();
	bool Render(ID3D11DeviceContext*, CameraClass*);
	bool Render(ID3D11DeviceContext* deviceContext);

	int GetIndexCount();

private:
//	bool LoadSetupFile(char*);
	void ShutdownHeightMap();
	void SetTerrainCoordinates();
	bool CalculateNormals();
	bool BuildTerrainModel();
	void ShutdownTerrainModel();

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*, CameraClass*);
	void RenderBuffers(ID3D11DeviceContext* deviceContext);
	bool UpdateBuffers(ID3D11DeviceContext*, CameraClass*);

	bool LoadDiamondSquareHeightMap();

private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;

	int m_terrainHeight, m_terrainWidth;
	float m_heightScale;
	char* m_terrainFilename;
	HeightMapType* m_heightMap;
	ModelType* m_terrainModel;
};

#endif