#include "Model.h"

Model::Model(const char* file, int voxelTextureSize)
{
	std::vector<GLfloat> voxelTextureData(voxelTextureSize * voxelTextureSize * voxelTextureSize, 0.0f);
	voxel_texture_ = new Texture3D(voxelTextureData, voxelTextureSize, voxelTextureSize, voxelTextureSize, true);
	// Make a JSON object
	std::string text = get_file_contents(file);
	JSON = json::parse(text);

	// Get the binary data
	Model::file = file;
	data = getData();

	// Traverse all nodes
	traverseNode(0);
}

void Model::Draw(Shader& shader, Camera& camera)
{	
	unsigned int j = 0;
	// Go over all meshes and draw each one
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		// Safety check to prevent out-of-bounds access
		//std::cout << meshes[i].textures[0].ID << "\n";
		meshes[i].Mesh::Draw(shader, camera, matricesMeshes[0]);
		
	}
}

void Model::DrawVoxels(Shader& shader, Camera& camera)
{
	unsigned int j = 0;
	// Go over all meshes and draw each one
	for (unsigned int i = 0; i < voxels.size(); i++)
	{
		// Safety check to prevent out-of-bounds access
		//std::cout << meshes[i].textures[0].ID << "\n";
		voxels[i].Voxelization::Draw(shader, voxel_texture_, camera, matricesMeshes[0]);
	}
}

void Model::VisualizeVoxels(Shader& shader, Camera& camera)
{
	unsigned int j = 0;
	// Go over all meshes and draw each one
	//for (unsigned int i = 0; i < voxels.size(); i++)
	//{
		// Safety check to prevent out-of-bounds access
		//std::cout << meshes[i].textures[0].ID << "\n";
		//voxels[0].Voxelization::visualizeVoxels(shader, voxel_texture_, camera);
		voxel_cone_tracing_->visualizeVoxels(shader, voxel_texture_, camera);
	//}
}	

void Model::loadMesh(unsigned int indMesh) {
	const json& mesh = JSON["meshes"][indMesh];
	const json& primitives = mesh["primitives"];

	std::vector<std::vector<Texture>> textures = getTextures();

	int voxelSize = 128;

	// Iterate over all primitives in the mesh
	for (const auto& primitive : primitives) {
		// Safely obtain accessor indices with fallbacks
		unsigned int posAccInd = primitive["attributes"].value("POSITION", 0);
		unsigned int normalAccInd = primitive["attributes"].value("NORMAL", 0);
		unsigned int texAccInd = primitive["attributes"].value("TEXCOORD_0", 0);
		unsigned int indAccInd = primitive.value("indices", 0);
		unsigned int materialInd = primitive.value("material", 0);

		// Use accessor indices to get all vertex components
		std::vector<float> posVec = getFloats(JSON["accessors"][posAccInd]);
		std::vector<glm::vec3> positions = groupFloatsVec3(posVec);
		std::vector<float> normalVec = normalAccInd > 0 ? getFloats(JSON["accessors"][normalAccInd]) : std::vector<float>();
		std::vector<glm::vec3> normals = !normalVec.empty() ? groupFloatsVec3(normalVec) : std::vector<glm::vec3>();
		std::vector<float> texVec = texAccInd > 0 ? getFloats(JSON["accessors"][texAccInd]) : std::vector<float>();
		std::vector<glm::vec2> texUVs = !texVec.empty() ? groupFloatsVec2(texVec) : std::vector<glm::vec2>();

		// Assemble vertices. Handle missing attributes by passing empty vectors where appropriate
		std::vector<Vertex> vertices = assembleVertices(positions, normals, texUVs);

		// Get indices and textures
		std::vector<GLuint> indices = indAccInd > 0 ? getIndices(JSON["accessors"][indAccInd]) : std::vector<GLuint>();

		// Combine the vertices, indices, and textures into a mesh and add to the model
		//meshes.push_back(Mesh(vertices, indices, textures[materialInd]));

		voxels.push_back(Voxelization(vertices, indices, textures[materialInd]));
	}

}




void Model::traverseNode(unsigned int nextNode, glm::mat4 matrix)
{
	// Current node
	json node = JSON["nodes"][nextNode];

	// Get translation if it exists
	glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
	if (node.find("translation") != node.end())
	{
		float transValues[3];
		for (unsigned int i = 0; i < node["translation"].size(); i++)
			transValues[i] = (node["translation"][i]);
		translation = glm::make_vec3(transValues);
	}
	// Get quaternion if it exists
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	if (node.find("rotation") != node.end())
	{
		float rotValues[4] =
		{
			node["rotation"][3],
			node["rotation"][0],
			node["rotation"][1],
			node["rotation"][2]
		};
		rotation = glm::make_quat(rotValues);
	}
	// Get scale if it exists
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	if (node.find("scale") != node.end())
	{
		float scaleValues[3];
		for (unsigned int i = 0; i < node["scale"].size(); i++)
			scaleValues[i] = (node["scale"][i]);
		scale = glm::make_vec3(scaleValues);
	}
	// Get matrix if it exists
	glm::mat4 matNode = glm::mat4(1.0f);
	if (node.find("matrix") != node.end())
	{
		float matValues[16];
		for (unsigned int i = 0; i < node["matrix"].size(); i++)
			matValues[i] = (node["matrix"][i]);
		matNode = glm::make_mat4(matValues);
	}

	// Initialize matrices
	glm::mat4 trans = glm::mat4(1.0f);
	glm::mat4 rot = glm::mat4(1.0f);
	glm::mat4 sca = glm::mat4(1.0f);

	// Use translation, rotation, and scale to change the initialized matrices
	trans = glm::translate(trans, translation);
	rot = glm::mat4_cast(rotation);
	sca = glm::scale(sca, scale);

	// Multiply all matrices together
	glm::mat4 matNextNode = matrix * matNode * trans * rot * sca;

	// Check if the node contains a mesh and if it does load it
		translationsMeshes.push_back(translation);
		rotationsMeshes.push_back(rotation);
		scalesMeshes.push_back(scale);
		matricesMeshes.push_back(matNextNode);

		loadMesh(node["mesh"]);

	// Check if the node has children, and if it does, apply this function to them with the matNextNode
	if (node.find("children") != node.end())
	{
		for (unsigned int i = 0; i < node["children"].size(); i++)
			traverseNode(node["children"][i], matNextNode);
	}
}

std::vector<unsigned char> Model::getData()
{
	// Create a place to store the raw text, and get the uri of the .bin file
	std::string bytesText;
	std::string uri = JSON["buffers"][0]["uri"];

	// Store raw text data into bytesText
	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);
	bytesText = get_file_contents((fileDirectory + uri).c_str());

	// Transform the raw text data into bytes and put them in a vector
	std::vector<unsigned char> data(bytesText.begin(), bytesText.end());
	return data;
}

std::vector<float> Model::getFloats(json accessor)
{
	std::vector<float> floatVec;

	// Get properties from the accessor
	unsigned int buffViewInd = accessor.value("bufferView", 1);
	unsigned int count = accessor["count"];
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	std::string type = accessor["type"];

	// Get properties from the bufferView
	json bufferView = JSON["bufferViews"][buffViewInd];
	unsigned int byteOffset = bufferView["byteOffset"];

	// Interpret the type and store it into numPerVert
	unsigned int numPerVert;
	if (type == "SCALAR") numPerVert = 1;
	else if (type == "VEC2") numPerVert = 2;
	else if (type == "VEC3") numPerVert = 3;
	else if (type == "VEC4") numPerVert = 4;
	else throw std::invalid_argument("Type is invalid (not SCALAR, VEC2, VEC3, or VEC4)");

	// Go over all the bytes in the data at the correct place using the properties from above
	unsigned int beginningOfData = byteOffset + accByteOffset;
	unsigned int lengthOfData = count * 4 * numPerVert;
	for (unsigned int i = beginningOfData; i < beginningOfData + lengthOfData; i)
	{
		unsigned char bytes[] = { data[i++], data[i++], data[i++], data[i++] };
		float value;
		std::memcpy(&value, bytes, sizeof(float));
		floatVec.push_back(value);
	}

	return floatVec;
}

std::vector<GLuint> Model::getIndices(const json accessor) {
	std::vector<GLuint> indices;

	// Get properties from the accessor
	unsigned int buffViewInd = accessor.value("bufferView", 0);
	unsigned int count = accessor["count"];
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	unsigned int componentType = accessor["componentType"];

	// Get properties from the bufferView
	const json& bufferView = JSON["bufferViews"][buffViewInd];
	unsigned int byteOffset = bufferView["byteOffset"];

	// Calculate the actual starting point of the data
	unsigned int dataStart = byteOffset + accByteOffset;

	// Determine the size of each index based on its component type
	size_t componentSize = 0;
	switch (componentType) {
	case 5125: componentSize = 4; break; // UNSIGNED_INT
	case 5123: componentSize = 2; break; // UNSIGNED_SHORT
	case 5122: componentSize = 2; break; // SHORT
	default: throw std::runtime_error("Unsupported component type for indices.");
	}

	// Loop through the data and extract indices
	for (unsigned int i = 0, offset = dataStart; i < count; ++i, offset += componentSize) {
		if (componentType == 5125) { // UNSIGNED_INT
			indices.push_back(*reinterpret_cast<const GLuint*>(&data[offset]));
		}
		else if (componentType == 5123 || componentType == 5122) { // UNSIGNED_SHORT or SHORT
			// Note: Assuming UNSIGNED_SHORT since indices should not be negative
			indices.push_back(*reinterpret_cast<const unsigned short*>(&data[offset]));
		}
	}

	return indices;
}


std::vector<std::vector<Texture>> Model::getTextures() {
	std::vector<std::vector<Texture>> textures;

	textures.resize(JSON["materials"].size());
	std::string fileDirectory = std::string(file).substr(0, std::string(file).find_last_of('/') + 1);

	for (size_t i = 0; i < JSON["materials"].size(); ++i) {
		const auto& material = JSON["materials"][i];

		if (material.contains("pbrMetallicRoughness")) {
			if (material["pbrMetallicRoughness"].contains("baseColorTexture")) {
				int texIndex = material["pbrMetallicRoughness"]["baseColorTexture"]["index"];
				loadTextureByIndex(texIndex, "diffuse", fileDirectory, textures[i]);
			}
			if (material["pbrMetallicRoughness"].contains("metallicRoughnessTexture")) {
				int texIndex = material["pbrMetallicRoughness"]["metallicRoughnessTexture"]["index"];
				loadTextureByIndex(texIndex, "metallicRoughness", fileDirectory, textures[i]);
			}
		}
		if (material.contains("normalTexture")) {
			int texIndex = material["normalTexture"]["index"];
			loadTextureByIndex(texIndex, "normal", fileDirectory, textures[i]);
		}
	}

	return textures;
}

void Model::loadTextureByIndex(int index, const char* type, const std::string& fileDirectory, std::vector<Texture>& textures) {
	int imageIndex  = findImageIndexByTextureIndex(index);
	std::string texPath = JSON["images"][imageIndex]["uri"];

	auto it = std::find(loadedTexName.begin(), loadedTexName.end(), texPath);
	if (it == loadedTexName.end()) {
		Texture texture((fileDirectory + texPath).c_str(), type, loadedTex.size());
		textures.push_back(texture);
		loadedTex.push_back(texture);
		loadedTexName.push_back(texPath);
	}
	else {
		textures.push_back(loadedTex[std::distance(loadedTexName.begin(), it)]);
	}
}

int Model::findImageIndexByTextureIndex(int index){
	const json& texture = JSON["textures"][index];
	int imageIndex = texture["source"];
	//imageTexCoordIndex = texture["texCoord"];
	return imageIndex;
}





std::vector<Vertex> Model::assembleVertices
(
	std::vector<glm::vec3> positions,
	std::vector<glm::vec3> normals,
	std::vector<glm::vec2> texUVs
)
{
	std::vector<Vertex> vertices;
	for (int i = 0; i < positions.size(); i++)
	{
		vertices.push_back
		(
			Vertex
			{
				positions[i],
				normals[i],
				glm::vec3(1.0f, 1.0f, 1.0f),
				texUVs[i]
			}
		);
	}
	return vertices;
}

std::vector<glm::vec2> Model::groupFloatsVec2(std::vector<float> floatVec)
{
	std::vector<glm::vec2> vectors;
	for (int i = 0; i < floatVec.size(); i)
	{
		vectors.push_back(glm::vec2(floatVec[i++], floatVec[i++]));
	}
	return vectors;
}
std::vector<glm::vec3> Model::groupFloatsVec3(std::vector<float> floatVec)
{
	std::vector<glm::vec3> vectors;
	for (int i = 0; i < floatVec.size(); i)
	{
		vectors.push_back(glm::vec3(floatVec[i++], floatVec[i++], floatVec[i++]));
	}
	return vectors;
}
std::vector<glm::vec4> Model::groupFloatsVec4(std::vector<float> floatVec)
{
	std::vector<glm::vec4> vectors;
	for (int i = 0; i < floatVec.size(); i)
	{
		vectors.push_back(glm::vec4(floatVec[i++], floatVec[i++], floatVec[i++], floatVec[i++]));
	}
	return vectors;
}
