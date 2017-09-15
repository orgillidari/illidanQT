#include "FbxLoader.h"

#define POLYGON_POINT_NUM 3  //������

XFbxLoader::XFbxLoader()
{
	m_pVertexInfo = 0;
}

XFbxLoader::~XFbxLoader()
{

}

void XFbxLoader::Init()
{
	m_Manager = FbxManager::Create();
	FbxIOSettings* pIoSettings = FbxIOSettings::Create(m_Manager, "FbxIOSetttings");
	m_Manager->SetIOSettings(pIoSettings);
	m_Importer = FbxImporter::Create(m_Manager, "FbxImporter");
}

int XFbxLoader::LoadFbxFile(const char* pFbxFile)
{
	m_Importer->Initialize(pFbxFile, -1, m_Manager->GetIOSettings());
	FbxScene* pScene = FbxScene::Create(m_Manager, pFbxFile);
	bool res = m_Importer->Import(pScene);

	if (!res)
		return -1;

	FbxNode* pRootNode = pScene->GetRootNode();
	ProcessNode(pRootNode);

	pScene->Destroy();
	return 0;
}

int XFbxLoader::ProcessMaterial(FbxNode* pNode)
{
	int nMaterialCount = pNode->GetMaterialCount();
	for (int i = 0; i < nMaterialCount; ++i)
	{
		FbxSurfaceMaterial* pSurfaceMaterial = pNode->GetMaterial(i);

		int type = 0;
		if (pSurfaceMaterial->GetClassId() == FbxSurfaceLambert::ClassId)
		{
			type = 1;
		}
		else if (pSurfaceMaterial->GetClassId() == FbxSurfacePhong::ClassId)
		{
			type = 2;
		}
		else
		{
			return -1;
		}

		FbxProperty oProperty = pSurfaceMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[0]);
		if (oProperty.IsValid())
		{
			int nSrcCount = oProperty.GetSrcObjectCount<FbxTexture>();
			if (nSrcCount < 1)
				return -2;
			FbxTexture* oTexture = oProperty.GetSrcObject<FbxTexture>(0);

			int index = -1;
			int nAllMatsCount = m_vAllMats.size();
 			for (int j = 0; j < nAllMatsCount; ++j)
			{
				if (m_vAllMats[j].type == type && strcmp(m_vAllMats[j].texture, oTexture->GetName()) == 0)
				{
					index = j;
					break;
				}
			}

			if (index == -1)
			{
				index = m_vAllMats.size();
				Mat mat;
				memset(&mat, 0, sizeof(Mat));
				mat.type = type;
				strcpy(mat.texture, oTexture->GetName());

				//GetTextureName("./Resources/Model/tauren.fbx", oTexture->GetName(), mat.texture);
				m_vAllMats.push_back(mat);
			}

			m_uMatsIndex.insert(std::pair<int, int>(i, index));
		}
	}

	m_vIndices.resize(m_vAllMats.size());

	return 0;
}

int XFbxLoader::ProcessMesh(FbxNode* pNode)
{
	FbxMesh* pMesh = pNode->GetMesh();
	if (!pMesh)
		return -1;

	int nControlPointsCount = pMesh->GetControlPointsCount();
	FbxVector4* pControlPoints = pMesh->GetControlPoints();

	int nPolygonCount = pMesh->GetPolygonCount();

	if (m_pVertexInfo)
	{
		delete[] m_pVertexInfo;
		m_pVertexInfo = 0;
	}

	m_PVertexSize = nPolygonCount * POLYGON_POINT_NUM;
	m_pVertexInfo = new XVertexInfo[m_PVertexSize];
	
	memset(m_pVertexInfo, 0, m_PVertexSize * sizeof(XVertexInfo));

	for (int i = 0; i < nPolygonCount; ++i)
	{
		int nPolygonSize = pMesh->GetPolygonSize(i);
		if (nPolygonSize != POLYGON_POINT_NUM)
		{
			return -2;
		}
		for (int j = 0; j < nPolygonSize; ++j)
		{
			//������������
			int nVertexIndex = pMesh->GetPolygonVertex(i, j);
			FbxVector4 pos = pControlPoints[nVertexIndex];

			m_pVertexInfo[i * POLYGON_POINT_NUM + j].pos[0] = pos.mData[0];
			m_pVertexInfo[i * POLYGON_POINT_NUM + j].pos[1] = pos.mData[1];
			m_pVertexInfo[i * POLYGON_POINT_NUM + j].pos[2] = pos.mData[2];

			//������������
			FbxVector4 normal;
			if (pMesh->GetPolygonVertexNormal(i, j, normal))
			{
				m_pVertexInfo[i * POLYGON_POINT_NUM + j].normal[0] = normal.mData[0];
				m_pVertexInfo[i * POLYGON_POINT_NUM + j].normal[1] = normal.mData[1];
				m_pVertexInfo[i * POLYGON_POINT_NUM + j].normal[2] = normal.mData[2];
			}

			//����UV����
			FbxStringList oUVSetList;
			pMesh->GetUVSetNames(oUVSetList);
			if (oUVSetList.GetCount() > 1)
			{
				FbxVector2 uv;
				bool flag = true;
				if (pMesh->GetPolygonVertexUV(i, j, oUVSetList[0], uv, flag))
				{
					m_pVertexInfo[i * POLYGON_POINT_NUM + j].uv[0] = uv.mData[0];
					m_pVertexInfo[i * POLYGON_POINT_NUM + j].uv[1] = uv.mData[1];
				}
			}
		}

		ProcessMaterialIndex(pMesh, i);
	}

	return 0;
}

int XFbxLoader::ProcessNode(FbxNode* pNode)
{
	int nAttributeCount = pNode->GetNodeAttributeCount();
	for (int i = 0; i < nAttributeCount; ++i)
	{
		FbxNodeAttribute* pAttribute = pNode->GetNodeAttributeByIndex(i);

		if (!pAttribute)
			continue;

		FbxNodeAttribute::EType eType = pAttribute->GetAttributeType();
		switch (eType)
		{
		case FbxNodeAttribute::eMesh:
		{
			ProcessMaterial(pNode);
			ProcessMesh(pNode);
			break;
		}
		}
	}

	//�����ӽڵ�
	int nChildCount = pNode->GetChildCount();
	for (int i = 0; i < nChildCount; ++i)
	{
		FbxNode* pChild = pNode->GetChild(i);
		ProcessNode(pChild);
	}

	return 0;
}

int XFbxLoader::ProcessNormal(FbxMesh* pMesh)
{


	return 0;
}

int XFbxLoader::ProcessUV(FbxMesh* pMesh)
{

	return 0;
}

int XFbxLoader::ProcessMaterialIndex(FbxMesh* pMesh, int index)
{
	int nElementMaterialCount = pMesh->GetElementMaterialCount();
	if (nElementMaterialCount < 1)
		return -1;

	FbxGeometryElementMaterial* pGeometryElementMaterial = pMesh->GetElementMaterial(0);
	if (!pGeometryElementMaterial)
		return -2;

	FbxLayerElement::EMappingMode eMappingMode = pGeometryElementMaterial->GetMappingMode();
	FbxLayerElement::EReferenceMode eReferenceMode = pGeometryElementMaterial->GetReferenceMode();
	switch (eMappingMode)
	{
	case FbxLayerElement::eByPolygon:
	{
		switch (eReferenceMode)
		{
		case FbxLayerElement::eIndexToDirect:
			{
				int nMaterialIndex = pGeometryElementMaterial->GetIndexArray().GetAt(index);
				int nMapIndex = m_uMatsIndex[nMaterialIndex];
				m_vIndices[nMapIndex].push_back(index * 3 + 0);
				m_vIndices[nMapIndex].push_back(index * 3 + 1);
				m_vIndices[nMapIndex].push_back(index * 3 + 2);
			}
		}
		break;
	}
	}
	return 0;
}

int XFbxLoader::GetTextureName(const char* pModelFile, const char* pFileName, char* pBuff)
{
	strcpy(pBuff, pModelFile);
	memset(pBuff + strlen(pBuff) - 3, 0, 3);
	strcat(pBuff, "fbm/");
	strcat(pBuff, pFileName);

	return 0;
}