#include "scene.h"
#include "scenerunner.h"
#include "scenenormalmap.h"
#include "scenefog.h"
#include "scenemultilight.h"
#include "sceneperfragment.h"
#include "scenespot.h"
#include "scenetoon.h"
#include "scenepbr.h"
#include "scenestripe.h"
#include "scenelatti.h"
#include "scenepervertex.h"

std::map<std::string, std::string> sceneInfo = {
	{ "normal-map", "Add bump texture" },
	{ "fog", "Fog" },
	{ "multi-light", "Multiple light sources" },
	{ "per-frag", "Per-fragment shading" },
	{ "per-vertex", "Per-vertex shading" },
	{ "spot", "Spot light" },
	{ "toon", "Toon shading" },
	{ "pbr", "Physically based rendering (PBR) shader"},
	{ "phong", "Using separable shaders and program pipelines"},
	{ "stripe",  " stripe vertex shader"},
	{ "lattice", "draw geometry with holes"}

};


int main(int argc, char *argv[])
{
	std::string recipe = SceneRunner::parseCLArgs(argc, argv, sceneInfo);

	SceneRunner runner("Chapter 4 - " + recipe);
	std::unique_ptr<Scene> scene;
	if( recipe == "normal-map" ) {
		scene = std::unique_ptr<Scene>( new SceneNormalMap() );
	} else if( recipe == "fog") {
		scene = std::unique_ptr<Scene>( new SceneFog() );
	} else if( recipe == "multi-light") {
		scene = std::unique_ptr<Scene>( new SceneMultiLight() );
	} else if( recipe == "per-frag" ) {
		scene = std::unique_ptr<Scene>( new ScenePerFragment() );
	} else if( recipe == "spot") {
		scene = std::unique_ptr<Scene>( new SceneSpot() );
	} else if( recipe == "toon" ) {
		scene = std::unique_ptr<Scene>( new SceneToon() );
	} else if (recipe == "pbr") {
		scene = std::unique_ptr<Scene>(new ScenePbr());
	}else if (recipe == "lattice") {
		scene = std::unique_ptr<Scene>(new SceneLatti() );
	} else if (recipe == "per-vertex") {
		scene = std::unique_ptr<Scene>(new ScenePervertex());
	}else if (recipe == "stripe") {
		scene = std::unique_ptr<Scene>(new SceneStripe());
	}
	return runner.run(std::move(scene));
}
