#pragma once

#define TGL_TARGET_GL_4_5
#include <scene/scene_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>

#include <map>
#include <vector>

#include "Shader.h"

class MyView : public tygra::WindowViewDelegate
{
public:

    MyView();
    ~MyView();

public:
    void SetScene(const scene::Context * scene);

private:

    void windowViewWillStart(tygra::Window * window) override;

    void windowViewDidReset(tygra::Window * window,
                            int width,
                            int height) override;

    void windowViewDidStop(tygra::Window * window) override;

    void windowViewRender(tygra::Window * window) override;

	GLuint GetTexture(std::string);
	
	void CreatePointLights() const;
	void DrawLights();
	const scene::Context * mScene;
	

	struct Vertex
	{
		scene::Vector3 position;
		scene::Vector3 normal;
		scene::Vector3 tangent;
		scene::Vector2 texcoord;
	};

	struct PerInstanceData
	{
		glm::mat4 modelXform;
		glm::uint materialID;
	};

	struct Mesh
	{
		GLuint vertexVbo{ 0 };
		GLuint elementVbo{ 0 };
		GLuint vao{ 0 };
		int elementCount{ 0 };
	};
	struct DrawElementsIndirectCommand
	{
		GLuint count;
		GLuint primCount;
		GLuint firstIndex;
		GLuint baseVertex;
		GLuint baseInstance;
	};
	struct Material
	{
		GLuint diffuse_texture_index;
		GLuint normal_texture_index;
		GLuint roughness_texture_index;
		GLuint metalic_texture_index;
	};

	struct DirectionalLight
	{
		glm::vec3 direction;
		float pad;
		glm::vec3 intensity;
		float pad2;
	};

	struct PointLight
	{
		glm::vec3 position;
		float pad;
		glm::vec3 intensity;
		float range;
	};

	struct SpotLight
	{
		glm::vec3 position;
		float pad;
		glm::vec3 intensity;
		float pad2;
		glm::vec3 direction;
		float range;
		float coneAngle;
		float pad3;
		glm::vec2 pad4;
	};

	struct ShadowData
	{
		GLuint frameBuffer{ 0 };
		GLuint shadowMap{ 0 };
		glm::mat4 lightMatrix;
	};

	GLuint m_vertex_vbo;
	GLuint m_instance_vbo;
	GLuint m_element_vbo;
	GLuint m_indirect_idb;
	GLuint m_material_ubo;
	GLuint m_vao;

	Mesh m_lightQuad;
	Mesh m_lightSphere;

	//Light UBO's
	GLuint m_directional_ubo{ 0 };
	GLuint m_point_ubo{ 0 };
	GLuint m_spot_ubo{ 0 };

	GLuint m_gAlbedoMetalBuffer = 0;
	GLuint m_gPositionBuffer = 0;
	GLuint m_gNormalRoughnessBuffer = 0;
	GLuint m_zBuffer = 0;
	GLuint m_gBuffer = 0;
	GLuint m_lBuffer = 0;
	GLuint m_lColorBuffer = 0;
	GLuint m_textureArray = 0;


	Shader geometryPass;
	Shader lightingDirPass;
	Shader lightingPointPass;
	Shader lightingSpotPass;

	unsigned int m_meshCount{ 0 };

	int width_{ 0 };
	int height_{ 0 };

	std::map<scene::MaterialId, Material> materials_;
	std::map<std::string, GLuint> textures_;

};
