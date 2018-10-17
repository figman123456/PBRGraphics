#include "MyView.hpp"
#include <scene/scene.hpp>
#include <tygra/FileHelper.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cassert>
#include <regex>

MyView::MyView()
{
}

MyView::~MyView() {
}

void MyView::SetScene(const scene::Context * scene)
{
    mScene = scene;
}

void MyView::windowViewWillStart(tygra::Window * window)
{
    assert(mScene != nullptr);
	geometryPass = Shader("reprise_vs.glsl", "reprise_fs.glsl");
	lightingDirPass = Shader("lightingPass_vs.glsl", "lightingPass_fs.glsl");
	lightingPointPass = Shader("lightingPointPass_vs.glsl", "lightingPointPass_fs.glsl");
	lightingSpotPass = Shader("lightingSpotPass_vs.glsl", "lightingSpotPass_fs.glsl");

	lightingDirPass.Use();
	glUniform1i(glGetUniformLocation(lightingDirPass.Program, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(lightingDirPass.Program, "gNormalRoughness"), 1);
	glUniform1i(glGetUniformLocation(lightingDirPass.Program, "gAlbedoMetal"), 2);

	lightingPointPass.Use();
	glUniform1i(glGetUniformLocation(lightingPointPass.Program, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(lightingPointPass.Program, "gNormalRoughness"), 1);
	glUniform1i(glGetUniformLocation(lightingPointPass.Program, "gAlbedoMetal"), 2);

	lightingSpotPass.Use();
	glUniform1i(glGetUniformLocation(lightingSpotPass.Program, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(lightingSpotPass.Program, "gNormalRoughness"), 1);
	glUniform1i(glGetUniformLocation(lightingSpotPass.Program, "gAlbedoMetal"), 2);
	{
		std::vector<glm::vec2> vertices(4);
		vertices[0] = glm::vec2(-1, -1);
		vertices[1] = glm::vec2(1, -1);
		vertices[2] = glm::vec2(1, 1);
		vertices[3] = glm::vec2(-1, 1);

		glGenBuffers(1, &m_lightQuad.vertexVbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_lightQuad.vertexVbo);
		glBufferData(GL_ARRAY_BUFFER,
			vertices.size() * sizeof(glm::vec2),
			vertices.data(),
			GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenVertexArrays(1, &m_lightQuad.vao);
		glBindVertexArray(m_lightQuad.vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_lightQuad.vertexVbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
			sizeof(glm::vec2), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	scene::GeometryBuilder builder;
	const auto& scene_meshes = builder.getAllMeshes();
	const auto& scene_instances = mScene->getAllInstances();

	std::vector<Vertex> vertices;
	std::vector<unsigned int> elements;
	std::vector<glm::mat4> transforms;
	std::vector<DrawElementsIndirectCommand> drawCommands;

	int baseInstaceIndex = 0;

	for (const auto& scene_mesh : scene_meshes)
	{

		const auto& positions = scene_mesh.getPositionArray();
		const auto& elementsArray = scene_mesh.getElementArray();
		const auto& normals = scene_mesh.getNormalArray();
		const auto& tangents = scene_mesh.getTangentArray();
		const auto& texCoords = scene_mesh.getTextureCoordinateArray();



		DrawElementsIndirectCommand cmd;
		cmd.count = elementsArray.size();
		cmd.primCount = mScene->getInstancesByMeshId(scene_mesh.getId()).size();
		cmd.firstIndex = elements.size();
		cmd.baseVertex = vertices.size();
		cmd.baseInstance = baseInstaceIndex;
		baseInstaceIndex += mScene->getInstancesByMeshId(scene_mesh.getId()).size();
		drawCommands.push_back(cmd);
		m_meshCount++;

		for (unsigned int i = 0; i < positions.size(); i++)
		{
			Vertex newVertex;
			newVertex.position = positions[i];
			newVertex.normal = normals[i];
			if (tangents.size() > 0)
			{
				newVertex.tangent = tangents[i];
				newVertex.texcoord = texCoords[i];
			}
			else
			{
				newVertex.tangent = scene::Vector3(0.0f,0.0f,0.0f);
				newVertex.texcoord = scene::Vector2(0.0f,0.0f);
			}

			vertices.push_back(newVertex);
		}

		elements.insert(elements.end(), elementsArray.begin(), elementsArray.end());

	}



	glGenBuffers(1, &m_vertex_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_vbo);
	glBufferData(GL_ARRAY_BUFFER,
		vertices.size() * sizeof(Vertex),
		vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_element_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_element_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		elements.size() * sizeof(unsigned int),
		elements.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_instance_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
	glBufferData(GL_ARRAY_BUFFER, scene_instances.size() * sizeof(PerInstanceData), nullptr, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_indirect_idb);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirect_idb);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, drawCommands.size() * sizeof(DrawElementsIndirectCommand), drawCommands.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_element_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		sizeof(Vertex), TGL_BUFFER_OFFSET_OF(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
		sizeof(Vertex), TGL_BUFFER_OFFSET_OF(Vertex, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
		sizeof(Vertex), TGL_BUFFER_OFFSET_OF(Vertex, tangent));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE,
		sizeof(Vertex), TGL_BUFFER_OFFSET_OF(Vertex, texcoord));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
	glEnableVertexAttribArray(6);
	glVertexAttribIPointer(6, 1, GL_UNSIGNED_INT, sizeof(PerInstanceData), TGL_BUFFER_OFFSET_OF(PerInstanceData, materialID));
	glVertexAttribDivisor(6, 1);
	for (unsigned int i = 0; i < 4; i++) 
	{
		glEnableVertexAttribArray(7 + i);
		glVertexAttribPointer(7 + i, 4, GL_FLOAT, GL_FALSE, sizeof(PerInstanceData), reinterpret_cast<const GLvoid*>(sizeof(GLfloat) * i * 4));
		glVertexAttribDivisor(7 + i, 1);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	geometryPass.Use();

	std::vector<Material> materialData;
	for (unsigned int i = 0; i < 7*4;i+=4)
	{
		Material mat;
		mat.diffuse_texture_index = i;
		mat.normal_texture_index = i + 1;
		mat.roughness_texture_index = i + 2;
		mat.metalic_texture_index = i + 3;
		materialData.push_back(mat);
	}

	glGenBuffers(1, &m_material_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, m_material_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Material) * materialData.size(), materialData.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_material_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	const std::string front("content:///");
	const std::string albedo("_albedo.png");
	const std::string normal("_normal.png");
	const std::string rough("_rough.png");
	const std::string metal("_metal.png");

	glGenTextures(1, &m_textureArray);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 6, GL_RGBA8, 1024, 1024, 4 * materialData.size());
	GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };

	int textureLoad = 0;
	for(unsigned int i = 0; i < materialData.size()*4; i+=4)
	{
		std::ostringstream index;
		index << textureLoad;
		std::string test = front + index.str() + albedo;
		tygra::Image albedo_image = tygra::createImageFromPngFile(front + index.str() + albedo);
		if (albedo_image.doesContainData()) 
		{
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, 1024, 1024, 1, pixel_formats[albedo_image.componentsPerPixel()], albedo_image.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE
				: GL_UNSIGNED_SHORT, albedo_image.pixelData());
		}
		tygra::Image normal_image = tygra::createImageFromPngFile(front + index.str() + normal);
		if (normal_image.doesContainData()) 
		{
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i+1, 1024, 1024, 1, pixel_formats[normal_image.componentsPerPixel()], normal_image.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE
				: GL_UNSIGNED_SHORT, normal_image.pixelData());
		}
		tygra::Image rough_image = tygra::createImageFromPngFile(front + index.str() + rough);
		if (rough_image.doesContainData()) 
		{
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i+2, 1024, 1024, 1, pixel_formats[rough_image.componentsPerPixel()], rough_image.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE
				: GL_UNSIGNED_SHORT, rough_image.pixelData());
		}
		tygra::Image metal_image = tygra::createImageFromPngFile(front + index.str() + metal);
		if (metal_image.doesContainData()) 
		{
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i+3, 1024, 1024, 1, pixel_formats[metal_image.componentsPerPixel()], metal_image.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE
				: GL_UNSIGNED_SHORT, metal_image.pixelData());
		}
		textureLoad++;
	}

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glUniform1i(glGetUniformLocation(geometryPass.Program,"textureArray"),0);
	glUseProgram(0);

	lightingDirPass.Use();

	std::vector<DirectionalLight> directional_lights;

	for(const auto light : mScene->getAllDirectionalLights())
	{
		DirectionalLight dLight;
		dLight.direction = (const glm::vec3&)light.getDirection();
		dLight.intensity = (const glm::vec3&)light.getIntensity();
		directional_lights.push_back(dLight);
	}



	glGenBuffers(1, &m_directional_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, m_directional_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(DirectionalLight) * directional_lights.size(), directional_lights.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_directional_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenBuffers(1, &m_point_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, m_point_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PointLight) * mScene->getAllPointLights().size(), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_point_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenBuffers(1, &m_spot_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, m_spot_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SpotLight) * mScene->getAllSpotLights().size(), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 4, m_spot_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void MyView::windowViewDidReset(tygra::Window * window,
                                int width,
                                int height)
{
	width_ = width;
	height_ = height;
    glViewport(0, 0, width, height);

	glGenFramebuffers(1, &m_gBuffer);
	glGenFramebuffers(1, &m_lBuffer);
	
	glGenTextures(1, &m_gPositionBuffer);
	glBindTexture(GL_TEXTURE_2D, m_gPositionBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	glGenTextures(1, &m_gNormalRoughnessBuffer);
	glBindTexture(GL_TEXTURE_2D, m_gNormalRoughnessBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &m_gAlbedoMetalBuffer);
	glBindTexture(GL_TEXTURE_2D, m_gAlbedoMetalBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &m_lColorBuffer);
	glBindTexture(GL_TEXTURE_2D, m_lColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenRenderbuffers(1, &m_zBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_zBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gPositionBuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gNormalRoughnessBuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gAlbedoMetalBuffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_zBuffer);

	GLenum buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, buffers);

	GLuint frambuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (frambuffer_status != GL_FRAMEBUFFER_COMPLETE)
		tglDebugMessage(GL_DEBUG_SEVERITY_HIGH_ARB, "GBuffer not complete!");

	glBindFramebuffer(GL_FRAMEBUFFER, m_lBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_lColorBuffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_zBuffer);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	frambuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (frambuffer_status != GL_FRAMEBUFFER_COMPLETE)
		tglDebugMessage(GL_DEBUG_SEVERITY_HIGH_ARB, "LBuffer not complete!");
	

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

}

void MyView::windowViewDidStop(tygra::Window * window)
{
}

void MyView::windowViewRender(tygra::Window * window)
{
	assert(mScene != nullptr);
#ifdef DEBUG
	nvtxRangePushA("EntireDraw");
#endif // DEBUG

	const auto& camera_ = mScene->getCamera();
	glm::mat4 projection_xform = glm::perspective(glm::radians(camera_.getVerticalFieldOfViewInDegrees()),
		static_cast<float>(width_) / height_,
		camera_.getNearPlaneDistance(),
		camera_.getFarPlaneDistance());
	glm::mat4 view_xform = glm::lookAt((const glm::vec3&)camera_.getPosition(), (const glm::vec3&)camera_.getPosition() + (const glm::vec3&)camera_.getDirection(), (const glm::vec3&)mScene->getUpDirection());
	glm::vec3 cameraPos = (const glm::vec3&)camera_.getPosition();

	const auto instances = mScene->getAllInstances();
	std::vector<PerInstanceData> model_xforms;
	for (const auto instance : instances)
	{
		PerInstanceData data;
		data.materialID = instance.getMaterialId() - 200;
		data.modelXform = glm::mat4((const glm::mat4x3&)instance.getTransformationMatrix());
		model_xforms.push_back(data);
	}

	const auto pointLights = mScene->getAllPointLights();
	std::vector<PointLight> pointLightData;
	for(const auto pointLight : pointLights)
	{
		PointLight pLight;
		pLight.position = (const glm::vec3&)pointLight.getPosition();
		pLight.intensity = (const glm::vec3&)pointLight.getIntensity();
		pLight.range = pointLight.getRange();
		pointLightData.push_back(pLight);
	}

	const auto spotLights = mScene->getAllSpotLights();
	std::vector<SpotLight> spotLightData;
	for (const auto spotLight : spotLights)
	{
		SpotLight sLight;
		sLight.position = (const glm::vec3&)spotLight.getPosition();
		sLight.intensity = (const glm::vec3&)spotLight.getIntensity();
		sLight.direction = (const glm::vec3&)spotLight.getDirection();
		sLight.range = spotLight.getRange();
		sLight.coneAngle = spotLight.getConeAngleDegrees();
		spotLightData.push_back(sLight);
	}

	glBindBuffer(GL_UNIFORM_BUFFER, m_point_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PointLight) * pointLightData.size(), pointLightData.data());
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBuffer(GL_UNIFORM_BUFFER, m_spot_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SpotLight) * spotLightData.size(), spotLightData.data());
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(PerInstanceData) * model_xforms.size(), model_xforms.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

#ifdef DEBUG
	nvtxRangePushA("GBuffer");
#endif // DEBUG
	glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
	{	
		glViewport(0, 0, width_, height_);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthMask(GL_TRUE);
		
		geometryPass.Use();
		glUniformMatrix4fv(glGetUniformLocation(geometryPass.Program, "projection_xform"), 1, GL_FALSE, glm::value_ptr(projection_xform));
		glUniformMatrix4fv(glGetUniformLocation(geometryPass.Program, "view_xform"), 1, GL_FALSE, glm::value_ptr(view_xform));
		glBindVertexArray(m_vao);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirect_idb);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, m_meshCount, sizeof(DrawElementsIndirectCommand));
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
		glBindVertexArray(0);
	}
	


	glDisable(GL_DEPTH_TEST);

	glEnable(GL_FRAMEBUFFER_SRGB);
	glBindFramebuffer(GL_FRAMEBUFFER, m_lBuffer);
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
#ifdef DEBUG
	nvtxRangePop();
	nvtxRangePushA("DirectionalLights");
#endif // DEBUG
		lightingDirPass.Use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_gPositionBuffer);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_gNormalRoughnessBuffer);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_gAlbedoMetalBuffer);

		glUniform3fv(glGetUniformLocation(lightingDirPass.Program, "camera_position"), 1, glm::value_ptr(cameraPos));

		glBindVertexArray(m_lightQuad.vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glBindVertexArray(0);

#ifdef DEBUG
	nvtxRangePop();
	nvtxRangePushA("Point");
#endif // DEBUG

		lightingPointPass.Use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_gPositionBuffer);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_gNormalRoughnessBuffer);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_gAlbedoMetalBuffer);

		glUniform3fv(glGetUniformLocation(lightingPointPass.Program, "camera_position"), 1, glm::value_ptr(cameraPos));

		glBindVertexArray(m_lightQuad.vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glBindVertexArray(0);

#ifdef DEBUG
	nvtxRangePop();
	nvtxRangePushA("Spot");
#endif // DEBUG

		lightingSpotPass.Use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_gPositionBuffer);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_gNormalRoughnessBuffer);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_gAlbedoMetalBuffer);

		glUniform3fv(glGetUniformLocation(lightingSpotPass.Program, "camera_position"), 1, glm::value_ptr(cameraPos));

		glBindVertexArray(m_lightQuad.vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glBindVertexArray(0);
		glDisable(GL_BLEND);

	}

	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_lBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, width_, height_, 0, 0, width_, height_, GL_COLOR_BUFFER_BIT ,GL_NEAREST);

#ifdef DEBUG
	nvtxRangePop();
	nvtxRangePop();
#endif // DEBUG
}

GLuint MyView::GetTexture(const std::string l_filePath)
{
	//If the filepaths blank get the GLuint of the white texture
	if (l_filePath == "")
	{
		return GetTexture("content:///hex.png");
	}

	//Iterator of the found texture if not found equal to the end of the map
	auto it = textures_.find(l_filePath);

	//If the texture isn't found create it and add ith to the map
	if (it == textures_.end())
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		GLuint newTexture{ 0 };
		tygra::Image texture_image = tygra::createImageFromPngFile(l_filePath);
		if (texture_image.doesContainData()) {
			glGenTextures(1, &newTexture);
			glBindTexture(GL_TEXTURE_2D, newTexture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
			glTexImage2D(GL_TEXTURE_2D,
				0,
				GL_RGBA,
				texture_image.width(),
				texture_image.height(),
				0,
				pixel_formats[texture_image.componentsPerPixel()],
				texture_image.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE
				: GL_UNSIGNED_SHORT,
				texture_image.pixelData());
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		textures_[l_filePath] = newTexture;
		return newTexture;
	}
	else // Get the GLuint of the exsisting texture.
	{
		return it->second;
	}
}

void MyView::CreatePointLights() const
{
}

void MyView::DrawLights()
{	
}