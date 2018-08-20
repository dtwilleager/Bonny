#include "stdafx.h"
#include "ModelLoader.h"

#include <assimp/material.h>
#include <assimp/matrix4x4.h>
#include <assimp/vector3.h>
#include <assimp/quaternion.h>

#include <IL\il.h>
#include <atlstr.h>

using std::make_shared;
using std::static_pointer_cast;

using glm::mat4;
using glm::vec3;
using glm::dquat;

namespace Bonny
{
  ModelLoader::ModelLoader()
  {
    ilInit();
  }


  ModelLoader::~ModelLoader()
  {
  }

  shared_ptr<Entity> ModelLoader::loadAssimpModel(string filename)
  {
    Assimp::Importer importer;
    shared_ptr<Entity> rootEntity = NULL;
    shared_ptr<Mesh> rlMesh;
    shared_ptr<Material> rlMaterial;
    shared_ptr<RenderComponent> renderComponent;

    //const aiScene* scene = importer.ReadFile(filename.c_str(),
    //  aiProcess_CalcTangentSpace |
    //  aiProcess_Triangulate |
    //  aiProcess_JoinIdenticalVertices |
    //  aiProcess_SortByPType |
    //  aiProcess_GenSmoothNormals);

    uint32_t AssimpFlags = aiProcessPreset_TargetRealtime_MaxQuality |
      aiProcess_OptimizeGraph |
      aiProcess_CalcTangentSpace |
      //aiProcess_FlipUVs |
      // aiProcess_FixInfacingNormals | // causes incorrect facing normals for crytek-sponza
      0;
    //AssimpFlags &= ~(aiProcess_CalcTangentSpace);

    const aiScene* scene = importer.ReadFile(filename.c_str(), AssimpFlags);

    // If the import failed, report it
    if (!scene)
    {
      const char* error = importer.GetErrorString();
      return NULL;
    }

    unsigned int numMeshes = scene->mNumMeshes;
    unsigned int numMaterials = scene->mNumMaterials;

    rootEntity = make_shared<Entity>(scene->mRootNode->mName.C_Str());
    if (scene->mRootNode->mNumMeshes > 0)
    {
      // Create Entity for this node
      renderComponent = make_shared<RenderComponent>(scene->mRootNode->mName.C_Str());

      for (unsigned int i = 0; i<scene->mRootNode->mNumMeshes; i++)
      {
        aiMesh* mesh = scene->mMeshes[scene->mRootNode->mMeshes[i]];
        unsigned int numVerts = mesh->mNumVertices;
        unsigned int numBuffers = 1;

        if (mesh->HasNormals())
        {
          numBuffers++;
        }

        if (mesh->HasTextureCoords(0))
        {
          numBuffers++;
        }

        if (mesh->HasTangentsAndBitangents())
        {
          numBuffers += 2;
        }

        rlMesh = make_shared<Mesh>(scene->mRootNode->mName.C_Str(), Mesh::TRIANGLES, numVerts, numBuffers);
        int texIndex = 0;
        aiString texturePath;
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        if (material->GetTexture(aiTextureType_DIFFUSE, texIndex, &texturePath) == AI_SUCCESS)
        {
          int normalIndex = 0;
          aiString normalPath;
          if (material->GetTexture(aiTextureType_HEIGHT, normalIndex, &normalPath) == AI_SUCCESS && texturePath != normalPath)
          {
            rlMaterial = make_shared<Material>("ModelMaterial", Material::LIT);
          }
          else
          {
            rlMaterial = make_shared<Material>("ModelMaterial", Material::LIT);
          }
        }
        else
        {
          rlMaterial = make_shared<Material>("ModelMaterial", Material::LIT_NOTEXTURE);
          if (material->GetTexture(aiTextureType_HEIGHT, texIndex, &texturePath) == AI_SUCCESS)
          {
            printLog("UNLIT NORMAL MAP");
          }
        }
        populateMeshMaterial(scene, rlMesh, rlMaterial, mesh);
        rlMesh->setMaterial(rlMaterial);
        renderComponent->addMesh(rlMesh);
      }
      rootEntity->addComponent(renderComponent);
    }
    mat4 transform = glm::make_mat4((float*)&(scene->mRootNode->mTransformation));
    rootEntity->setTransform(transform);

    // Process the children
    for (unsigned int i = 0; i<scene->mRootNode->mNumChildren; i++)
    {
      processNode(scene, rootEntity, scene->mRootNode->mChildren[i]);
    }

    printLog("Done Loaded Mesh");

    return (rootEntity);
  }

  void ModelLoader::processNode(const aiScene* scene, shared_ptr<Entity> parent, aiNode* node)
  {
    shared_ptr<Entity> entity = NULL;
    shared_ptr<Mesh> rlMesh;
    shared_ptr<Material> rlMaterial;
    shared_ptr<RenderComponent> renderComponent;

    entity = make_shared<Entity>(node->mName.C_Str());
    if (node->mNumMeshes > 0)
    {
      renderComponent = make_shared<RenderComponent>(node->mName.C_Str());

      for (unsigned int i = 0; i<node->mNumMeshes; i++)
      {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        unsigned int numVerts = mesh->mNumVertices;
        unsigned int numBuffers = 1;

        if (mesh->HasNormals())
        {
          numBuffers++;
        }

        if (mesh->HasTextureCoords(0))
        {
          numBuffers++;
        }

        if (mesh->HasTangentsAndBitangents())
        {
          numBuffers += 2;
        }

        rlMesh = make_shared<Mesh>(node->mName.C_Str(), Mesh::TRIANGLES, numVerts, numBuffers);
        printLog("Loaded Mesh: " + std::to_string(numVerts));
        int texIndex = 0;
        aiString texturePath;
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        if (material->GetTexture(aiTextureType_DIFFUSE, texIndex, &texturePath) == AI_SUCCESS)
        {
          int normalIndex = 0;
          aiString normalPath;
          if (material->GetTexture(aiTextureType_HEIGHT, normalIndex, &normalPath) == AI_SUCCESS && texturePath != normalPath)
          {
            rlMaterial = make_shared<Material>("ModelMaterial", Material::LIT);
          }
          else
          {
            rlMaterial = make_shared<Material>("ModelMaterial", Material::LIT);
          }
        }
        else
        {
          rlMaterial = make_shared<Material>("ModelMaterial", Material::LIT_NOTEXTURE);
          if (material->GetTexture(aiTextureType_HEIGHT, texIndex, &texturePath) == AI_SUCCESS)
          {
            printLog("UNLIT NORMAL MAP");
          }
        }

        populateMeshMaterial(scene, rlMesh, rlMaterial, mesh);
        rlMesh->setMaterial(rlMaterial);
        renderComponent->addMesh(rlMesh);
      }
      entity->addComponent(renderComponent);
    }
    mat4 transform = glm::make_mat4((float*)&(node->mTransformation));
    entity->setTransform(transform);
    parent->addChild(entity);

    // Process the children
    for (unsigned int i = 0; i<node->mNumChildren; i++)
    {
      processNode(scene, entity, node->mChildren[i]);
    }
  }

  void ModelLoader::populateMeshMaterial(const aiScene* scene, shared_ptr<Mesh> rlMesh, shared_ptr<Material> rlMaterial, aiMesh* mesh)
  {
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    unsigned int numFaces = mesh->mNumFaces;
    unsigned int numVerts = mesh->mNumVertices;
    unsigned int bufferIndex = 0;

    float* verts = new float[numVerts * 3];
    unsigned int vindex = 0;
    for (unsigned int i = 0; i<numVerts; i++)
    {
      verts[vindex++] = mesh->mVertices[i].x;
      verts[vindex++] = mesh->mVertices[i].y;
      verts[vindex++] = mesh->mVertices[i].z;
    }
    rlMesh->addVertexBuffer(bufferIndex++, 3, sizeof(float)*numVerts * 3, verts);

    if (mesh->HasNormals())
    {
      float* normals = new float[numVerts * 3];
      unsigned int nindex = 0;
      for (unsigned int i = 0; i<numVerts; i++)
      {
        normals[nindex++] = mesh->mNormals[i].x;
        normals[nindex++] = mesh->mNormals[i].y;
        normals[nindex++] = mesh->mNormals[i].z;
      }
      rlMesh->addVertexBuffer(bufferIndex++, 3, sizeof(float)*numVerts * 3, normals);
    }

    if (mesh->HasTextureCoords(0))
    {
      float* texCoords = new float[numVerts * 2];
      unsigned int tindex = 0;
      for (unsigned int i = 0; i<numVerts; i++)
      {
        texCoords[tindex++] = mesh->mTextureCoords[0][i].x;
        texCoords[tindex++] = mesh->mTextureCoords[0][i].y;
      }
      rlMesh->addVertexBuffer(bufferIndex++, 2, sizeof(float)*numVerts * 2, texCoords);
    }

    if (mesh->HasTangentsAndBitangents())
    {
      float* tangents = new float[numVerts * 3];
      float* bitangents = new float[numVerts * 3];
      unsigned int tbindex = 0;
      for (unsigned int i = 0; i<numVerts; i++)
      {
        tangents[tbindex] = mesh->mTangents[i].x;
        bitangents[tbindex++] = mesh->mBitangents[i].x;
        tangents[tbindex] = mesh->mTangents[i].y;
        bitangents[tbindex++] = mesh->mBitangents[i].y;
        tangents[tbindex] = mesh->mTangents[i].z;
        bitangents[tbindex++] = mesh->mBitangents[i].z;
      }
      rlMesh->addVertexBuffer(bufferIndex++, 3, sizeof(float)*numVerts * 3, tangents);
      rlMesh->addVertexBuffer(bufferIndex++, 3, sizeof(float)*numVerts * 3, bitangents);
    }

    unsigned int* indexBuffer = new unsigned int[numFaces * 3];
    unsigned int iindex = 0;
    for (unsigned int i = 0; i<numFaces; i++)
    {
      const struct aiFace* face = &mesh->mFaces[i];
      indexBuffer[iindex++] = face->mIndices[0];
      indexBuffer[iindex++] = face->mIndices[1];
      indexBuffer[iindex++] = face->mIndices[2];
    }
    rlMesh->addIndexBuffer(numFaces * 3, indexBuffer);

    for (unsigned int i = 0; i<material->mNumProperties;)
    {
      aiMaterialProperty* property = material->mProperties[i];
      i++;
    }

    aiString name;

    glm::vec3 color;
    glm::vec4 color4;
    aiColor3D aColor;
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, aColor) == AI_SUCCESS)
    {
      color4.r = aColor.r;
      color4.g = aColor.g;
      color4.b = aColor.b;
      color4.a = 1.0f;
      rlMaterial->setAlbedoColor(color4);
    }

    if (material->Get(AI_MATKEY_COLOR_EMISSIVE, aColor) == AI_SUCCESS)
    {
      color.r = aColor.r;
      color.g = aColor.g;
      color.b = aColor.b;
      rlMaterial->setEmissiveColor(color);
    }

    int twoSided;
    if (material->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS)
    {
      rlMaterial->setTwoSided((twoSided == 0 ? false : true));
    }

    int wireframe;
    if (material->Get(AI_MATKEY_ENABLE_WIREFRAME, wireframe) == AI_SUCCESS)
    {
      rlMaterial->setTwoSided((wireframe == 0 ? false : true));
    }

    int texIndex = 0;
    aiString texturePath;
    if (material->GetTexture(aiTextureType_DIFFUSE, texIndex, &texturePath) == AI_SUCCESS)
    {
      shared_ptr<Texture> texture = loadTexture((const char*)texturePath.C_Str());
      rlMaterial->setAlbedoTexture(texture);
      printLog("Loaded Texture: " + std::string(texturePath.C_Str()));
    }

    int normalIndex = 0;
    aiString normalPath;
    if (material->GetTexture(aiTextureType_HEIGHT, normalIndex, &normalPath) == AI_SUCCESS && texturePath != normalPath)
    {
      shared_ptr<Texture> normalTexture = loadTexture((const char*)normalPath.C_Str());
      rlMaterial->setNormalTexture(normalTexture);
      printLog("Loaded Normal Texture: " + std::string(normalPath.C_Str()));
    }

    //int metallicIndex = 0;
    //aiString metallicPath;
    //if (material->GetTexture(aiTextureType_AMBIENT, metallicIndex, &metallicPath) == AI_SUCCESS && texturePath != metallicPath)
    //{
    //  shared_ptr<Texture> metallicTexture = loadTexture((const char*)metallicPath.C_Str());
    //  rlMaterial->setMetallicTexture(metallicTexture);
    //  printLog("Loaded Metallic Texture: " + std::string(metallicPath.C_Str()));
    //}

    int roughnessIndex = 0;
    aiString roughnessPath;
    if (material->GetTexture(aiTextureType_SHININESS, roughnessIndex, &roughnessPath) == AI_SUCCESS && texturePath != roughnessPath)
    {
      shared_ptr<Texture> roughnessTexture = loadTexture((const char*)roughnessPath.C_Str());
      rlMaterial->setMetallicRoughnessTexture(roughnessTexture);
      printLog("Loaded Roughness Texture: " + std::string(roughnessPath.C_Str()));
    }
  }

  shared_ptr<Texture> ModelLoader::loadTexture(const char* filename)
  {
    ILuint ilDiffuseID;
    ILboolean success;
    shared_ptr<Texture> texture = nullptr;

    map<string, shared_ptr<Texture>>::iterator it = m_textureMap.find(filename);
    if (it != m_textureMap.end())
    {
      //element found;
      return it->second;
    }

    /* generate DevIL Image IDs */
    ilGenImages(1, &ilDiffuseID);
    ilBindImage(ilDiffuseID); /* Binding of DevIL image name */
    success = ilLoadImage((const char*)filename);
    //ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

    if (success) /* If no error occured: */
    {
      success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
      if (!success)
      {
        return NULL;
      }

      texture = make_shared<Texture>(filename, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), ilGetInteger(IL_IMAGE_DEPTH),
        ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL), ilGetInteger(IL_IMAGE_SIZE_OF_DATA), ilGetInteger(IL_IMAGE_FORMAT));
      texture->setData(ilGetData());

      m_textureMap[filename] = texture;
    }

    return texture;
  }

  void ModelLoader::printLog(string s)
  {
    string st = s + "\n";
    TCHAR name[256];
    _tcscpy_s(name, CA2T(st.c_str()));
    OutputDebugString(name);
  }
}
