////////////////////////////////////////////////////////////////////////////////
// Filename: terrainclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "terrainclass.h"


TerrainClass::TerrainClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_terrainFilename = 0;
	m_heightMap = 0;
	m_terrainModel = 0;
}


TerrainClass::TerrainClass(const TerrainClass& other)
{
}


TerrainClass::~TerrainClass()
{
}

bool TerrainClass::Initialize(ID3D11Device* device)
{
	bool result;

	m_heightScale = 12.0;
	m_terrainHeight = m_terrainWidth = 257;
	result = LoadDiamondSquareHeightMap();
	if (!result)
	{
		return false;
	}

	// Setup the X and Z coordinates for the height map as well as scale the terrain height by the height scale value.
	SetTerrainCoordinates();

	// Now build the 3D model of the terrain.
	result = BuildTerrainModel();
	if (!result)
	{
		return false;
	}

	// We can now release the height map since it is no longer needed in memory once the 3D terrain model has been built.
	//ShutdownHeightMap();

	// Load the rendering buffers with the terrain data.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	// Release the terrain model now that the rendering buffers have been loaded.
	ShutdownTerrainModel();

	return true;
}

void TerrainClass::Shutdown()
{
	// Release the rendering buffers.
	ShutdownBuffers();

	// Release the terrain model.
	ShutdownTerrainModel();

	// Release the height map.
	ShutdownHeightMap();


	return;
}


bool TerrainClass::Render(ID3D11DeviceContext* deviceContext, CameraClass* camera)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext, camera);

	return true;
}


int TerrainClass::GetIndexCount()
{
	return m_indexCount;
}


bool TerrainClass::LoadDiamondSquareHeightMap()
{
	int i, j, index;

	m_heightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
	if (!m_heightMap)
	{
		return false;
	}

	DiamondSquare ds(m_terrainWidth, 50, 0, 0);
	double** map = ds.process();

	// Read the image data into the height map array.
	for (j = 0; j < m_terrainHeight; j++)
	{
		for (i = 0; i < m_terrainWidth; i++)
		{
			// Bitmaps are upside down so load bottom to top into the height map array.
			index = (m_terrainWidth * (m_terrainHeight - 1 - j)) + i;

			m_heightMap[index].y = map[j][i] - 200; // should be 0 < x < 120

		}
	}
	
	return true;
}

void TerrainClass::ShutdownHeightMap()
{
	// Release the height map array.
	if (m_heightMap)
	{
		delete[] m_heightMap;
		m_heightMap = 0;
	}

	return;
}

void TerrainClass::SetTerrainCoordinates()
{
	int i, j, index;


	// Loop through all the elements in the height map array and adjust their coordinates correctly.
	for (j = 0; j < m_terrainHeight; j++)
	{
		for (i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainWidth * j) + i;

			// Set the X and Z coordinates.
			m_heightMap[index].x = (float)i;
			m_heightMap[index].z = -(float)j;

			// Move the terrain depth into the positive range.  For example from (0, -256) to (256, 0).
			m_heightMap[index].z += (float)(m_terrainHeight - 1);

			// Scale the height.
			m_heightMap[index].y /= m_heightScale;
		}
	}

	return;
}

bool TerrainClass::BuildTerrainModel()
{
	int i, j, index, index1, index2, index3, index4;


	// Calculate the number of vertices in the 3D terrain model.
	m_vertexCount = (m_terrainHeight - 1) * (m_terrainWidth - 1) * 6;

	// Create the 3D terrain model array.
	m_terrainModel = new ModelType[m_vertexCount];
	if (!m_terrainModel)
	{
		return false;
	}

	// Initialize the index into the height map array.
	index = 0;

	// Load the 3D terrain model with the height map terrain data.
	// We will be creating 2 triangles for each of the four points in a quad.
	for (j = 0; j < (m_terrainHeight - 1); j++)
	{
		for (i = 0; i < (m_terrainWidth - 1); i++)
		{
			// Get the indexes to the four points of the quad.
			index1 = (m_terrainWidth * j) + i;          // Upper left.
			index2 = (m_terrainWidth * j) + (i + 1);      // Upper right.
			index3 = (m_terrainWidth * (j + 1)) + i;      // Bottom left.
			index4 = (m_terrainWidth * (j + 1)) + (i + 1);  // Bottom right.

															// Now create two triangles for that quad.
															// Triangle 1 - Upper left.
			m_terrainModel[index].x = m_heightMap[index1].x;
			m_terrainModel[index].y = m_heightMap[index1].y;
			m_terrainModel[index].z = m_heightMap[index1].z;
			index++;

			// Triangle 1 - Upper right.
			m_terrainModel[index].x = m_heightMap[index2].x;
			m_terrainModel[index].y = m_heightMap[index2].y;
			m_terrainModel[index].z = m_heightMap[index2].z;
			index++;

			// Triangle 1 - Bottom left.
			m_terrainModel[index].x = m_heightMap[index3].x;
			m_terrainModel[index].y = m_heightMap[index3].y;
			m_terrainModel[index].z = m_heightMap[index3].z;
			index++;

			// Triangle 2 - Bottom left.
			m_terrainModel[index].x = m_heightMap[index3].x;
			m_terrainModel[index].y = m_heightMap[index3].y;
			m_terrainModel[index].z = m_heightMap[index3].z;
			index++;

			// Triangle 2 - Upper right.
			m_terrainModel[index].x = m_heightMap[index2].x;
			m_terrainModel[index].y = m_heightMap[index2].y;
			m_terrainModel[index].z = m_heightMap[index2].z;
			index++;

			// Triangle 2 - Bottom right.
			m_terrainModel[index].x = m_heightMap[index4].x;
			m_terrainModel[index].y = m_heightMap[index4].y;
			m_terrainModel[index].z = m_heightMap[index4].z;
			index++;
		}
	}

	return true;
}

void TerrainClass::ShutdownTerrainModel()
{
	// Release the terrain model data.
	if (m_terrainModel)
	{
		delete[] m_terrainModel;
		m_terrainModel = 0;
	}

	return;
}

bool TerrainClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	XMFLOAT4 color;

	/*
	// Set the color of the terrain grid.
	color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Calculate the number of vertices in the terrain.
	m_vertexCount = (m_terrainWidth - 1) * (m_terrainHeight - 1) * 6;

	// Set the index count to the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	int LOD = 50;
	int index = 0;
	// Load the vertex array and index array with 3D terrain model data.
	for (int i = 0; i < m_terrainWidth - 1 - i / LOD; i += i / LOD + 1)
	{
		for (int j = 0; j < m_terrainHeight - j / LOD - 1; j += j / LOD + 1)
		{
			int iLod = (i % LOD) * (i + 1);
			int jLod = (j % LOD) * (j + 1);
			// Get the indexes to the four points of the quad.
			int index1 = (m_terrainWidth * j) + i;          // Upper left.
			int index2 = (m_terrainWidth * j) + (i + i / LOD + 1);      // Upper right.
			int index3 = (m_terrainWidth * (j + j / LOD + 1)) + i;      // Bottom left.
			int index4 = (m_terrainWidth * (j + j / LOD + 1)) + (i + i / LOD + 1);  // Bottom right.

															// Now create two triangles for that quad.
															// Triangle 1 - Upper left.
			vertices[index].position = XMFLOAT3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
			vertices[index].color = color;
			indices[index] = index++;

			vertices[index].position = XMFLOAT3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
			vertices[index].color = color;
			indices[index] = index++;

			vertices[index].position = XMFLOAT3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
			vertices[index].color = color;
			indices[index] = index++;

			vertices[index].position = XMFLOAT3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
			vertices[index].color = color;
			indices[index] = index++;

			vertices[index].position = XMFLOAT3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
			vertices[index].color = color;
			indices[index] = index++;

			vertices[index].position = XMFLOAT3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
			vertices[index].color = color;
			indices[index] = index++;
		}
	}
	*/

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	//vertexData.pSysMem = vertices;
	//vertexData.SysMemPitch = 0;
	//vertexData.SysMemSlicePitch = 0;
	
	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, NULL, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	//indexData.pSysMem = indices;
	//indexData.SysMemPitch = 0;
	//indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, NULL, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the buffers have been created and loaded.
	//delete[] vertices;
	//vertices = 0;
	//
	//delete[] indices;
	//indices = 0;

	return true;
}


void TerrainClass::ShutdownBuffers()
{
	// Release the index buffer.
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}


void TerrainClass::RenderBuffers(ID3D11DeviceContext* deviceContext, CameraClass* camera)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	UpdateBuffers(deviceContext, camera);

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case lines.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

bool TerrainClass::UpdateBuffers(ID3D11DeviceContext* deviceContext, CameraClass* camera)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_MAPPED_SUBRESOURCE vertexData, indexData;
	XMFLOAT4 color;


	// Set the color of the terrain grid.
	color = XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f);

	// Calculate the number of vertices in the terrain.
	m_vertexCount = (m_terrainWidth - 1) * (m_terrainHeight - 1) * 6;

	// Set the index count to the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	XMFLOAT3 cameraPosition = camera->GetPosition();

	int LOD = 50;
	int index = 0;
	int step = 1;
	// Load the vertex array and index array with 3D terrain model data.
	for (int i = 0; i < m_terrainWidth - step; i += step)
	{
		
		for (int j = 0; j < m_terrainHeight - step; j += step)
		{
			step = max(i / LOD + 1, j / LOD + 1);
			// Get the indexes to the four points of the quad.
			int index1 = (m_terrainWidth * j) + i;          // Upper left.
			int index2 = (m_terrainWidth * j) + (i + step);      // Upper right.
			int index3 = (m_terrainWidth * (j + step)) + i;      // Bottom left.
			int index4 = (m_terrainWidth * (j + step)) + (i + step);  // Bottom right.
			

			vertices[index].position = XMFLOAT3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
			vertices[index].color = color;
			indices[index] = index++;

			vertices[index].position = XMFLOAT3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
			vertices[index].color = color;
			indices[index] = index++;

			vertices[index].position = XMFLOAT3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
			vertices[index].color = color;
			indices[index] = index++;

			vertices[index].position = XMFLOAT3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
			vertices[index].color = color;
			indices[index] = index++;

			vertices[index].position = XMFLOAT3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
			vertices[index].color = color;
			indices[index] = index++;

			vertices[index].position = XMFLOAT3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
			vertices[index].color = color;
			indices[index] = index++;
		}
	}

	//	Disable GPU access to the vertex buffer data.
	deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vertexData);
	//	Update the vertex buffer here.
	memcpy(vertexData.pData, vertices, sizeof(VertexType) * m_vertexCount);
	//	Reenable GPU access to the vertex buffer data.
	deviceContext->Unmap(m_vertexBuffer, 0);

	//	Disable GPU access to the vertex buffer data.
	deviceContext->Map(m_indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &indexData);
	//	Update the vertex buffer here.
	memcpy(indexData.pData, indices, sizeof(unsigned long) * m_indexCount);
	//	Reenable GPU access to the vertex buffer data.
	deviceContext->Unmap(m_indexBuffer, 0);

	// Release the arrays now that the buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

}