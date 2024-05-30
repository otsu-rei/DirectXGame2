#include "Model.h"

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
#include <Logger.h>

////////////////////////////////////////////////////////////////////////////////////////////
// Model Methods
////////////////////////////////////////////////////////////////////////////////////////////

void Model::Init(const std::string& directoryPath, const std::string& filename) {
	modelData_ = ModelMethods::LoadObjFile(directoryPath, filename);

	size_ = static_cast<uint32_t>(modelData_.meshs.size());

	// textureManagerでモデルで使うloadTextureを呼び出し
	for (auto& it : modelData_.materials) {
		if (it.isUseTexture) {
			MyEngine::GetTextureManager()->LoadTexture(it.textureFilePath);
		}
	}
}

void Model::Term() {

	for (uint32_t i = 0; i < size_; ++i) {
		// meshDataのdelete
		modelData_.meshs[i].vertexResource.reset();
		modelData_.meshs[i].indexResource.reset();

		// materialDataの終了処理
		if (modelData_.materials[i].isUseTexture) {
			MyEngine::GetTextureManager()->UnloadTexture(modelData_.materials[i].textureFilePath);
		}
	}

	// modelDataの削除
	modelData_.meshs.clear();
	modelData_.materials.clear();
}

ModelData ModelMethods::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData result;

	std::string mtlFilename;
	MaterialData materialData;

	std::string line;

	
	// VertexDataの一時保存
	std::vector<Vector4f> positions;
	std::vector<Vector2f> texcoords;
	std::vector<Vector3f> normals;

	std::vector<VertexData> vertexDatas; // vertexResourceに書き込み

	// faceの一時保存
	enum FaceType {
		v, vt, vn,

		kFaceTypeCount
	};

	// vertexdata
	uint32_t vertexDataIndexCount = 0;
	std::unordered_map<std::string, uint32_t> faces; // key: f "1/2/3", value: vertexDataの配列数

	// vertexDataIndexの調整
	uint32_t currentIndex[kFaceTypeCount] = { 0, 0, 0 };
	uint32_t startIndex[kFaceTypeCount] = { 1, 1, 1 };

	// indexdata
	std::vector<uint32_t> indexDatas; // indexResourceに書き込み用
	

	// Objファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	
	if (!file.is_open()) { //!< fileが見つからなかった.
		std::string errorLog;
		errorLog = "[Model Not Found] \n filePath: " + directoryPath + "/" + filename;

		Assert(false, errorLog, "Error: LoadObjFile");
	}

	while (std::getline(file, line)) { // fileから一列ずつ読み込み
		std::string identifire; // 識別子
		std::istringstream s(line);
		s >> identifire;

		if (identifire == "mtllib") { //!< マテリアルファイル名
			s >> mtlFilename; // ファイルnameの保存

		} else if (identifire == "o") {
			if (!positions.empty()) { //!< 二回目以降の"o"の場合
				// mesh一つ分の書き込みが終わったので保存
				MeshData meshData;
				meshData.vertexResource
					= std::make_unique<DxObject::BufferResource<VertexData>>(MyEngine::GetDevicesObj(), static_cast<uint32_t>(vertexDatas.size()));
				meshData.vertexResource->Memcpy(vertexDatas.data());

				meshData.indexResource
					= std::make_unique<DxObject::IndexBufferResource>(MyEngine::GetDevicesObj(), static_cast<uint32_t>(indexDatas.size()));
				meshData.indexResource->Memcpy(indexDatas.data());

				// meshとmaterialをmodelDataに格納
				result.meshs.push_back(std::move(meshData));
				result.materials.push_back(std::move(materialData));

				// 書き込みが終了したのでデータ初期化
				for (int i = 0; i < kFaceTypeCount; ++i) {
					// startIndexの更新
					startIndex[i] += currentIndex[i];
					currentIndex[i] = 0;
				}

				// データの初期化
				positions.clear();
				texcoords.clear();
				normals.clear();

				vertexDatas.clear();
				indexDatas.clear();

				faces.clear();
				vertexDataIndexCount = 0;
			}

		} else if (identifire == "v") { //!< vertex
			currentIndex[v]++;

			// positionに書き込み
			Vector4f position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;

			// 左手座標に変換
			position.z *= -1;

			// vectorに保存
			positions.push_back(position);
			
		} else if (identifire == "vt") { //!< texcoord
			currentIndex[vt]++;

			// texcoordに書き込み
			Vector2f texcoord;
			s >> texcoord.x >> texcoord.y;

			// 左手座標に変換
			texcoord.y = 1.0f - texcoord.y;

			// vectorに保存
			texcoords.push_back(texcoord);

		} else if (identifire == "vn") { //!< normal
			currentIndex[vn]++;

			// normalに書き込み
			Vector3f normal;
			s >> normal.x >> normal.y >> normal.z;

			// 左手座標に変換
			normal.z *= -1;

			// vectorに保存
			normals.push_back(normal);

		} else if (identifire == "usemtl") { //!< materialの使用名
			std::string usemtl;
			s >> usemtl;

			materialData = ModelMethods::LoadMaterailFile(directoryPath, mtlFilename, usemtl);

		} else if (identifire == "f") { //!< face 四角形ポリゴンに対応
			// (f) "1/2/3", "4/5/6", "7/8/9" ... と読み込む
			std::string faceStrings[4];
			s >> faceStrings[0] >> faceStrings[1] >> faceStrings[2] >> faceStrings[3];

			// vertexdata
			int vertexNum = 0;

			for (uint32_t i = 0; i < 4; ++i) {
				if (faceStrings[i] == "") { // 三角形ポリゴンなので i = 2 に break
					break;
				}

				// 頂点数を増加
				vertexNum++;

				// facesにすでにvertexdataがあるか確認
				auto it = faces.find(faceStrings[i]);
				if (it == faces.end()) { // ない場合, vertexdataの生成

					// faceStringからface番号を取得
					uint32_t faceNum[3] = { NULL, NULL, NULL };

					std::istringstream indexs(faceStrings[i]);

					for (int fi = 0; fi < 3; ++fi) {
						std::string index;
						std::getline(indexs, index, '/');

						if (index != "") {
							faceNum[fi] = std::stoi(index);
						}
					}

					// 各データの取り出し
					Vector4f position;
					position = positions.at(faceNum[v] - startIndex[v]);

					Vector2f texcoord = { 0.0f, 0.0f };
					if (faceNum[vt] != NULL) {
						texcoord = texcoords.at(faceNum[vt] - startIndex[vt]);
					}

					Vector3f normal;
					normal = normals.at(faceNum[vn] - startIndex[vn]);

					// vertexdataの生成
					VertexData vertexData = {
						position,
						texcoord,
						normal,
					};

					vertexDatas.push_back(vertexData);

					// facesにindex数の保存
					faces[faceStrings[i]] = vertexDataIndexCount;
					vertexDataIndexCount++;
				}
			}

			// indexdataの作成
			if (vertexNum == 3) { //!< 三角形ポリゴンの場合
				// 逆順にfacesでvertexIndexを問い合わせながらindexDetasに代入
				for (uint32_t i = vertexNum; i > 0; --i) {
					indexDatas.push_back(faces[faceStrings[i - 1]]);
				}

			} else if (vertexNum == 4) { //!< 四角形ポリゴンの場合
				// 三角形ポリゴンに変換
				// polygonA
				indexDatas.push_back(faces[faceStrings[0]]);
				indexDatas.push_back(faces[faceStrings[3]]);
				indexDatas.push_back(faces[faceStrings[1]]);

				// polygonB
				indexDatas.push_back(faces[faceStrings[3]]);
				indexDatas.push_back(faces[faceStrings[2]]);
				indexDatas.push_back(faces[faceStrings[1]]);

			} else {
				assert(false); //!< ポリゴンの頂点数の不足
			}
		}
	}

	file.close();

	// fileが終わったので最後のやつを初期化
	if (!positions.empty()) {
		// mesh一つ分の書き込みが終わったので保存
		MeshData meshData;
		meshData.vertexResource
			= std::make_unique<DxObject::BufferResource<VertexData>>(MyEngine::GetDevicesObj(), static_cast<uint32_t>(vertexDatas.size()));
		meshData.vertexResource->Memcpy(vertexDatas.data());

		meshData.indexResource
			= std::make_unique<DxObject::IndexBufferResource>(MyEngine::GetDevicesObj(), static_cast<uint32_t>(indexDatas.size()));
		meshData.indexResource->Memcpy(indexDatas.data());
		
		// meshとmaterialをmodelDataに格納
		result.meshs.push_back(std::move(meshData));
		result.materials.push_back(std::move(materialData));

		// 書き込みが終了したのでデータ初期化
		for (int i = 0; i < kFaceTypeCount; ++i) {
			// startIndexの更新
			startIndex[i] += currentIndex[i];
			currentIndex[i] = 0;
		}

		// 書き込みが終了したのでデータを終了処理
		positions.clear();
		texcoords.clear();
		normals.clear();

		vertexDatas.clear();
		indexDatas.clear();

		faces.clear();
	}

	return result;
}

MaterialData ModelMethods::LoadMaterailFile(const std::string& directoryPath, const std::string& filename, const std::string& usemtl) {
	MaterialData result;
	result.isUseTexture = false;

	std::string line;
	bool isMatchUsemtl = false;

	// ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open()); //!< ファイルが開けない

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "newmtl") { //!< usemtl name
			// usemtlと一致してるかの確認
			std::string newmtl;
			s >> newmtl;

			if (isMatchUsemtl) { //!< 見つかっていた場合
				break;
				
			} else { 
				if (usemtl == newmtl) { //!< 一致していた場合
					isMatchUsemtl = true;
				}
			}
		}

		if (!isMatchUsemtl) { // 使いたいマテリアルではないので飛ばす.
			continue;
		}

		// materail情報
		// identifierに応じた処理
		if (identifier == "map_Kd") { //!< 使うtextureの名前
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスに変換
			result.textureFilePath = directoryPath + "/" + textureFilename;
			result.isUseTexture = true;
			break;
		}
	}

	file.close();

	return result;
}
