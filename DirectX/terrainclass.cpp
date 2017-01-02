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
			m_terrainModel[index].tu = 0.0f;
			m_terrainModel[index].tv = 0.0f;
			index++;

			// Triangle 1 - Upper right.
			m_terrainModel[index].x = m_heightMap[index2].x;
			m_terrainModel[index].y = m_heightMap[index2].y;
			m_terrainModel[index].z = m_heightMap[index2].z;
			m_terrainModel[index].tu = 1.0f;
			m_terrainModel[index].tv = 0.0f;
			index++;

			// Triangle 1 - Bottom left.
			m_terrainModel[index].x = m_heightMap[index3].x;
			m_terrainModel[index].y = m_heightMap[index3].y;
			m_terrainModel[index].z = m_heightMap[index3].z;
			m_terrainModel[index].tu = 0.0f;
			m_terrainModel[index].tv = 1.0f;
			index++;

			// Triangle 2 - Bottom left.
			m_terrainModel[index].x = m_heightMap[index3].x;
			m_terrainModel[index].y = m_heightMap[index3].y;
			m_terrainModel[index].z = m_heightMap[index3].z;
			m_terrainModel[index].tu = 0.0f;
			m_terrainModel[index].tv = 1.0f;
			index++;

			// Triangle 2 - Upper right.
			m_terrainModel[index].x = m_heightMap[index2].x;
			m_terrainModel[index].y = m_heightMap[index2].y;
			m_terrainModel[index].z = m_heightMap[index2].z;
			m_terrainModel[index].tu = 1.0f;
			m_terrainModel[index].tv = 0.0f;
			index++;

			// Triangle 2 - Bottom right.
			m_terrainModel[index].x = m_heightMap[index4].x;
			m_terrainModel[index].y = m_heightMap[index4].y;
			m_terrainModel[index].z = m_heightMap[index4].z;
			m_terrainModel[index].tu = 1.0f;
			m_terrainModel[index].tv = 1.0f;
			index++;
		}
	}

	return true;
}

void TerrainClass::ShutdownTerrainModel()
{
	// Release the terrain model data.
/*	if (m_terrainModel)
	{
		delete[] m_terrainModel;
		m_terrainModel = 0;
	}
	*/
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
	XMFLOAT4 color2;


	// Set the color of the terrain grid.
	color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Calculate the number of vertices in the terrain.
	m_vertexCount = m_terrainWidth * m_terrainHeight;

	// Set the index count to the same as the vertex count.
	m_indexCount = m_vertexCount * 2;

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

	// Initialize the whole vertex array
	for (int i = 0; i < m_terrainWidth; i++) {
		for (int j = 0; j < m_terrainHeight; j++) {
			int HM_index = (m_terrainWidth * j) + i;
			vertices[HM_index].position = XMFLOAT3(m_heightMap[HM_index].x, m_heightMap[HM_index].y, m_heightMap[HM_index].z);
			vertices[HM_index].texture = XMFLOAT2(m_terrainModel[HM_index].tu, m_terrainModel[HM_index].tv);

			vertices[HM_index].color = color;
		}
	}

	int LOD = 100;
	int index = 0;
	int step = 2;
	int tmpStep;
	// Load the vertex array and index array with 3D terrain model data.
	for (int i = 0; i < m_terrainWidth - step; i += step)
	{
		// i is the index of the diagonal
		/*
			|
			o_ o_ o_ o <-- (i, i)
			|		 |
			o_ o_ o  o
			|	  |  |
			o_ o  o  o
			|\ |  |  |
			o_\o_ o_ o_ o_
			
			
		*/
		step = static_cast<int>(pow(2, i / LOD + 1));

		tmpStep = static_cast<int>(pow(2, (i + step) / LOD + 1));

		if (step == tmpStep) {

			for (int j = 0; j < i; j += step)
			{


				/*
				|
				o_ o_ o_ o
				|		 | <- draw this line
				o_ o_ o  o
				|	  |  |
				o_ o  o  o
				|  |  |  |
				o_ o_ o_ o_ o_

				No need to distinguish the different case, we draw all the triangle the same way:

				D ---- G ---- C
				| \  6 | \  8 |
				|  5 \ |  7 \ |
				H ---- I ---- F
				| \  2 | \  4 |
				| 1  \ |  3 \ |
				A ---- E ---- B

				*/

				// Get the indexes to the four points of the quad.
				int A = (m_terrainWidth * j) + i;
				int B = (m_terrainWidth * j) + (i + step);
				int D = (m_terrainWidth * (j + step)) + i;
				int C = (m_terrainWidth * (j + step)) + (i + step);

				int E = (A + B) / 2;
				int F = (B + C) / 2;
				int G = (C + D) / 2;
				int H = (A + D) / 2;

				int I = (m_terrainWidth * (j + step / 2)) + (i + step / 2);

				// Triangle 1
				indices[index++] = A;
				indices[index++] = E;
				indices[index++] = H;
				// Triangle 2
				indices[index++] = H;
				indices[index++] = E;
				indices[index++] = I;
				// Triangle 3
				indices[index++] = E;
				indices[index++] = B;
				indices[index++] = I;
				// Triangle 4
				indices[index++] = I;
				indices[index++] = B;
				indices[index++] = F;
				// Triangle 5
				indices[index++] = H;
				indices[index++] = I;
				indices[index++] = D;
				// Triangle 6
				indices[index++] = D;
				indices[index++] = I;
				indices[index++] = G;
				// Triangle 7
				indices[index++] = I;
				indices[index++] = F;
				indices[index++] = G;
				// Triangle 8
				indices[index++] = G;
				indices[index++] = F;
				indices[index++] = C;

				/*
				|
				o_ o_ o_ o
				|		 |
				o_ o_ o  o
				|	  |  |
				o_ o  o  o
				|\ |  |  |
				o_\o_ o_ o_ o_
				^
				|
				Draw column
				*/

				// Get the indexes to the four points of the quad.
				A = (m_terrainWidth * i) + j;
				B = (m_terrainWidth * i) + (j + step);
				D = (m_terrainWidth * (i + step)) + j;
				C = (m_terrainWidth * (i + step)) + (j + step);

				E = (A + B) / 2;
				F = (B + C) / 2;
				G = (C + D) / 2;
				H = (A + D) / 2;

				I = (m_terrainWidth * (i + step / 2)) + (j + step / 2);

				// Triangle 1
				indices[index++] = A;
				indices[index++] = E;
				indices[index++] = H;
				// Triangle 2
				indices[index++] = H;
				indices[index++] = E;
				indices[index++] = I;
				// Triangle 3
				indices[index++] = E;
				indices[index++] = B;
				indices[index++] = I;
				// Triangle 4
				indices[index++] = I;
				indices[index++] = B;
				indices[index++] = F;
				// Triangle 5
				indices[index++] = H;
				indices[index++] = I;
				indices[index++] = D;
				// Triangle 6
				indices[index++] = D;
				indices[index++] = I;
				indices[index++] = G;
				// Triangle 7
				indices[index++] = I;
				indices[index++] = F;
				indices[index++] = G;
				// Triangle 8
				indices[index++] = G;
				indices[index++] = F;
				indices[index++] = C;
			}

			int A = (m_terrainWidth * i) + i;
			int B = (m_terrainWidth * i) + (i + step);
			int D = (m_terrainWidth * (i + step)) + i;
			int C = (m_terrainWidth * (i + step)) + (i + step);

			int E = (A + B) / 2;
			int F = (B + C) / 2;
			int G = (C + D) / 2;
			int H = (A + D) / 2;

			int I = (m_terrainWidth * (i + step / 2)) + (i + step / 2);

			// Triangle 1
			indices[index++] = A;
			indices[index++] = E;
			indices[index++] = H;
			// Triangle 2
			indices[index++] = H;
			indices[index++] = E;
			indices[index++] = I;
			// Triangle 3
			indices[index++] = E;
			indices[index++] = B;
			indices[index++] = I;
			// Triangle 4
			indices[index++] = I;
			indices[index++] = B;
			indices[index++] = F;
			// Triangle 5
			indices[index++] = H;
			indices[index++] = I;
			indices[index++] = D;
			// Triangle 6
			indices[index++] = D;
			indices[index++] = I;
			indices[index++] = G;
			// Triangle 7
			indices[index++] = I;
			indices[index++] = F;
			indices[index++] = G;
			// Triangle 8
			indices[index++] = G;
			indices[index++] = F;
			indices[index++] = C;
		}
		else {

			for (int j = 0; j < i; j += step)
			{
				/*
				|
				o_ o_ o_ o
				|		 | <- draw this line
				o_ o_ o  o
				|	  |  |
				o_ o  o  o
				|  |  |  |
				o_ o_ o_ o_ o_

				D ----------- C
				| \    7    / |
				|  5 \   /  6 |
				H ---- I ---- F
				| \  2 | \  4 |
				| 1  \ |  3 \ |
				A ---- E ---- B

				*/

				// Get the indexes to the four points of the quad.
				int A = (m_terrainWidth * i) + j;
				int B = (m_terrainWidth * i) + (j + step);
				int D = (m_terrainWidth * (i + step)) + j;
				int C = (m_terrainWidth * (i + step)) + (j + step);

				int E = (A + B) / 2;
				int F = (B + C) / 2;
				int H = (A + D) / 2;

				int I = (m_terrainWidth * (i + step / 2)) + (j + step / 2);

				// Triangle 1
				indices[index++] = A;
				indices[index++] = E;
				indices[index++] = H;
				// Triangle 2
				indices[index++] = H;
				indices[index++] = E;
				indices[index++] = I;
				// Triangle 3
				indices[index++] = E;
				indices[index++] = B;
				indices[index++] = I;
				// Triangle 4
				indices[index++] = I;
				indices[index++] = B;
				indices[index++] = F;
				// Triangle 5
				indices[index++] = H;
				indices[index++] = I;
				indices[index++] = D;
				// Triangle 6
				indices[index++] = I;
				indices[index++] = F;
				indices[index++] = C;
				// Triangle 7
				indices[index++] = D;
				indices[index++] = I;
				indices[index++] = C;

				/*
				|
				o_ o_ o_ o
				|		 |
				o_ o_ o  o
				|	  |  |
				o_ o  o  o
				|\ |  |  |
				o_\o_ o_ o_ o_
				^
				|
				Draw column

				D ---- G ---- C
				| \  6 |  4 / |
				|  5 \ | /    |
				H ---- I    7 |
				| \  2 | \    |
				| 1  \ |  3 \ |
				A ---- E ---- B

				*/

				// Get the indexes to the four points of the quad.
				A = (m_terrainWidth * j) + i;
				B = (m_terrainWidth * j) + (i + step);
				D = (m_terrainWidth * (j + step)) + i;
				C = (m_terrainWidth * (j + step)) + (i + step);

				E = (A + B) / 2;
				int G = (C + D) / 2;
				H = (A + D) / 2;

				I = (m_terrainWidth * (j + step / 2)) + (i + step / 2);

				// Triangle 1
				indices[index++] = A;
				indices[index++] = E;
				indices[index++] = H;
				// Triangle 2
				indices[index++] = H;
				indices[index++] = E;
				indices[index++] = I;
				// Triangle 3
				indices[index++] = E;
				indices[index++] = B;
				indices[index++] = I;
				// Triangle 4
				indices[index++] = I;
				indices[index++] = C;
				indices[index++] = G;
				// Triangle 5
				indices[index++] = H;
				indices[index++] = I;
				indices[index++] = D;
				// Triangle 6
				indices[index++] = D;
				indices[index++] = I;
				indices[index++] = G;
				// Triangle 7
				indices[index++] = I;
				indices[index++] = B;
				indices[index++] = C;

			}
			/*
			D ----------- C
			| \    5    / |
			|  4 \   /    |
			H ---- I    6 |
			| \  2 | \    |
			| 1  \ |  3 \ |
			A ---- E ---- B

			*/

			int A = (m_terrainWidth * i) + i;
			int B = (m_terrainWidth * i) + (i + step);
			int D = (m_terrainWidth * (i + step)) + i;
			int C = (m_terrainWidth * (i + step)) + (i + step);

			int E = (A + B) / 2;
			int H = (A + D) / 2;

			int I = (m_terrainWidth * (i + step / 2)) + (i + step / 2);

			// Triangle 1
			indices[index++] = A;
			indices[index++] = E;
			indices[index++] = H;
			// Triangle 2
			indices[index++] = H;
			indices[index++] = E;
			indices[index++] = I;
			// Triangle 3
			indices[index++] = E;
			indices[index++] = B;
			indices[index++] = I;
			// Triangle 4
			indices[index++] = H;
			indices[index++] = I;
			indices[index++] = D;
			// Triangle 5
			indices[index++] = D;
			indices[index++] = I;
			indices[index++] = C;
			// Triangle 6
			indices[index++] = I;
			indices[index++] = B;
			indices[index++] = C;
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