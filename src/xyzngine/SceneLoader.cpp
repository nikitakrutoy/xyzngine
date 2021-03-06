#include "SceneLoader.h"
#include "OgreItem.h"
#include "OgreMesh2.h"
#include "RenderUtils.h"
#include "OgreCamera.h"
#include <fstream>

using json = nlohmann::json;

void StoreObjects(json& d, Ogre::SceneNode* node) {
	auto childs = node->getChildIterator();
	while (childs.hasMoreElements()) {
		Ogre::SceneNode* child = static_cast<Ogre::SceneNode*> (childs.getNext());
		auto childObj = json::object();
		childObj["name"] = child->getName();
		auto position = child->getPosition();
		auto scale = child->getScale();
		childObj["position"] = json::array({ position.x, position.y, position.z });
		childObj["scale"] = json::array({ scale.x, scale.y, scale.z });
		if (SceneLoader::getNodeScripts().find(child) != SceneLoader::getNodeScripts().end())
			childObj["scriptName"] = SceneLoader::getNodeScripts()[child];
			
		Ogre::SceneNode::ObjectIterator iter = child->getAttachedObjectIterator();
		while (iter.hasMoreElements()) {
			Ogre::MovableObject* movObj = iter.getNext();
			childObj["type"] = movObj->getMovableType();
			childObj["visible"] = movObj->isVisible();

			if (childObj["type"] == "Item") {
				Ogre::Item* item = dynamic_cast<Ogre::Item*>(movObj);
				childObj["meshName"] = item->getName();

				if (SceneLoader::getNodeMaterials().find(child) != SceneLoader::getNodeMaterials().end())
					childObj["materialName"] = SceneLoader::getNodeMaterials()[child];
			}
			if (childObj["type"] == "Light") {
				Ogre::Light* light = dynamic_cast<Ogre::Light*>(movObj);
				childObj["lightType"] = light->getType();
				auto dir = light->getDirection();
				childObj["direction"] = json::array({ dir.x, dir.y, dir.z });
			}
		}
		childObj["objects"] = json::array();
		StoreObjects(childObj["objects"], child);
		d.push_back(childObj);
	}
}


void LoadObjects(json& j, Ogre::SceneManager* sceneManager, EntityManager* entityManager, Ogre::SceneNode* node) {
	for (auto obj : j["objects"]) {
		Ogre::SceneNode* sceneNode = node->
			createChildSceneNode(Ogre::SCENE_DYNAMIC);
		sceneNode->setName(obj["name"]);
		auto position = obj["position"];
		auto scale = obj["scale"];
		float x = position[0];
		sceneNode->setPosition(position[0], position[1], position[2]);
		sceneNode->setScale(scale[0], scale[1], scale[2]);

		if (obj["type"] == "Item") {
			Ogre::Item* item = OgreUtils::loadMesh(obj["meshName"], sceneManager);

			item->setName(obj["meshName"]);
			if (obj.find("materialName") != obj.end()) {
				item->setDatablock(obj["materialName"]);
				SceneLoader::getNodeMaterials()[sceneNode] = obj["materialName"];
			};
			sceneNode->attachObject(item);
		}

		if (obj["type"] == "Light") {
			Ogre::Light* light = sceneManager->createLight();
			sceneNode->attachObject(light);
			light->setPowerScale(Ogre::Math::PI);
			light->setType(obj["lightType"]);
			auto dir = obj["direction"];
			light->setDirection(Ogre::Vector3(dir[0], dir[1], dir[2]));
		}
		if (obj.find("scriptName") != obj.end())
			SceneLoader::getNodeScripts()[sceneNode] = obj["scriptName"];

		if (obj.find("scriptName") != obj.end() && entityManager)
			entityManager->CreateEntity(sceneNode, obj["scriptName"]);
		LoadObjects(obj, sceneManager, entityManager, sceneNode);
	}
}

void SceneLoader::StoreJSON(Ogre::SceneManager* sceneManager, std::string path)
{
	json d = json::object();
	d["objects"] = json::array();
	StoreObjects(d["objects"], sceneManager->getRootSceneNode());
	auto cameraData = json::object();
	auto camera = sceneManager->getCameras().at(0);
	auto position = camera->getPosition();
	auto dir = camera->getDirection();
	cameraData["position"] = json::array({ position.x, position.y, position.z });
	cameraData["direction"] = json::array({ dir.x, dir.y, dir.z });
	cameraData["far"] = camera->getFarClipDistance();
	cameraData["near"] = camera->getNearClipDistance();
	d["cameraData"] = cameraData;
	std::ofstream o(path);
	o << std::setw(4) << d << std::endl;
}

void SceneLoader::LoadJSON(Ogre::SceneManager* sceneManager, EntityManager* entityManager, std::string path)
{
	sceneManager->clearScene(true);

	std::ifstream i(path);
	json j;
	i >> j;

	auto cameraIter = sceneManager->getCameraIterator();
	if (j.find("cameraData") != j.end()) {
		while (cameraIter.hasMoreElements()) {
			auto cameraData = j["cameraData"];
			auto m_pCamera = cameraIter.getNext();
			auto pos = cameraData["position"];
			auto dir = cameraData["direction"];
			m_pCamera->setPosition(Ogre::Vector3(pos[0], pos[1], pos[2]));
			m_pCamera->setDirection(Ogre::Vector3(dir[0], dir[1], dir[2]));
			m_pCamera->setNearClipDistance(cameraData["near"]);
			m_pCamera->setFarClipDistance(cameraData["far"]);
			m_pCamera->setAutoAspectRatio(false);
			m_pCamera->setFixedYawAxis(true);
		}
	}
	else {
		while (cameraIter.hasMoreElements()) {
			auto m_pCamera = cameraIter.getNext();
			m_pCamera->setPosition(150, 0, 0);
			m_pCamera->lookAt(0, 0, 0);
			m_pCamera->setNearClipDistance(0.2);
			m_pCamera->setFarClipDistance(2000);
			m_pCamera->setAutoAspectRatio(false);
			m_pCamera->setFixedYawAxis(true);
		}
	}

	
	LoadObjects(j, sceneManager, entityManager, sceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC));
}
