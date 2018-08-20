#pragma once

#include "Entity.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "View.h"

#include <string>
#include <vector>
#include <memory>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h> 
#include <assimp/postprocess.h>
#include <assimp/mesh.h>

using std::string;
using std::vector;
using std::shared_ptr;
using std::map;

namespace Bonny
{
	class ModelLoader
	{
	public:
		ModelLoader();
		~ModelLoader();

    shared_ptr<Entity>  loadAssimpModel(string filename);

  private:
    void processNode(const aiScene* scene, shared_ptr<Entity> parent, aiNode* node);
    void populateMeshMaterial(const aiScene* scene, shared_ptr<Mesh> rlMesh, shared_ptr<Material> rlMaterial, aiMesh* mesh);
    shared_ptr<Texture> loadTexture(const char* filename);

    void printLog(string s);
    map<string, shared_ptr<Texture>>          m_textureMap;
	};
}

