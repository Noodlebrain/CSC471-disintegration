/*
ZJ Wood CPE 471 Lab 3 base code
*/

#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <glad/glad.h>

#include "stb_image.h"

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "OpenVRclass.h"

#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;
shared_ptr<Shape> shape;
float eyeconvergence = 0.55;		// convergence point
float eyedistance = 0.09;		//3D intesity effec
double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	glm::mat4 viewM;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		if (w == 1)
		{
			speed = 1*ftime;
		}
		else if (s == 1)
		{
			speed = -1*ftime;
		}
		float yangle=0;
		if (a == 1)
			yangle = -1*ftime;
		else if(d==1)
			yangle = 1*ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed,1);
		dir = dir*R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R*T;
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our dragons to OpenGL
	GLuint VertexBufferID, VertexTexID,
		VertexNorIDFlat, VertexNorIDSmooth;

	//texture data
	GLuint Texture;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			eyeconvergence+=0.1;
			cout << eyeconvergence << endl;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			eyeconvergence -= 0.1;
			if(eyeconvergence<0)
				eyeconvergence = 0;
			cout << eyeconvergence << endl;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
		
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			eyedistance += 0.01;
			cout << eyedistance << endl;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
		
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			eyedistance -= 0.01;
			if (eyedistance < 0)
				eyedistance = 0;
			cout << eyedistance << endl;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			
		}
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;

			//change this to be the points converted to WORLD
			//THIS IS BROKEN< YOU GET TO FIX IT - yay!
			newPt[0] = 0;
			newPt[1] = 0;

			std::cout << "converted:" << newPt[0] << " " << newPt[1] << std::endl;
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
			//update the vertex array with the updated points
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*6, sizeof(float)*2, newPt);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		int width, height, channels;
		char filepath[1000];

		string resourceDirectory = "../resources" ;
		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/alduin.obj");
		shape->resize();
		shape->init();

		/* dragon texture */
		string str = resourceDirectory + "/alduin.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//set the texture to the correct samplers in the fragment shader:
		GLuint TexLocation = glGetUniformLocation(prog->pid, "tex");
		// Then bind the uniform sampler to texture units:
		glUseProgram(prog->pid);
		glUniform1i(TexLocation, 0);

		/* generate VAO for the dragon */
		glGenVertexArrays(1, &VertexArrayID);
		/* bind to focus on this VAO */
		glBindVertexArray(VertexArrayID);

		/* generate a position VBO for the dragon */
		glGenBuffers(1, &VertexBufferID);
		/* bind to focus on this VBO */
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
		
		/* use the loaded information from the OBJ to make pos/tex buffer */
		GLfloat *alduinPos = new GLfloat[shape->eleBuf[0].size() * 3];
		GLfloat *alduinTex = new GLfloat[shape->eleBuf[0].size() * 2];
		/* fill the position buffer according to the OBJ's index/element buffer */
		int i;
		for (i = 0; i < shape->eleBuf[0].size(); i++)
		{
			unsigned int index = shape->eleBuf[0][i];
			alduinPos[3 * i] = shape->posBuf->at(3 * index); /* x pos */
			alduinPos[3 * i + 1] = shape->posBuf->at(3 * index + 1); /* y pos */
			alduinPos[3 * i + 2] = shape->posBuf->at(3 * index + 2); /* z pos */

			alduinTex[2 * i] = shape->texBuf->at(2 * index); /* tex x pos */
			alduinTex[2 * i + 1] = shape->texBuf->at(2 * index + 1); /* tex y pos */
		}
		
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat) * shape->eleBuf[0].size(),
			alduinPos, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
		
		/* generate texture VBO*/
		glGenBuffers(1, &VertexTexID);
		glBindBuffer(GL_ARRAY_BUFFER, VertexTexID);
		/* copy data from our tex buffer to VBO*/
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat) * shape->eleBuf[0].size(),
			alduinTex, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);


		/* generate VBO for flat normals */
		glGenBuffers(1, &VertexNorIDFlat);
		glBindBuffer(GL_ARRAY_BUFFER, VertexNorIDFlat);

		GLfloat *alduinNorFlat = new GLfloat[3 * shape->eleBuf[0].size()];
		/* calculate face normals per triangle,
		 * and apply to each vertex of said triangle */
		for (i = 0; i < shape->eleBuf[0].size(); i += 3)
		{
			/* get point A and put it in a vec3 */
			unsigned int index = shape->eleBuf[0][i];
			GLfloat Ax = shape->posBuf->at(3 * index);
			GLfloat Ay = shape->posBuf->at(3 * index + 1);
			GLfloat Az = shape->posBuf->at(3 * index + 2);
			vec3 A(Ax, Ay, Az);
			/* get point B and put it in a vec3 */
			index = shape->eleBuf[0][i + 1];
			GLfloat Bx = shape->posBuf->at(3 * index);
			GLfloat By = shape->posBuf->at(3 * index + 1);
			GLfloat Bz = shape->posBuf->at(3 * index + 2);
			vec3 B(Bx, By, Bz);
			/* get point C and put it in a vec3 */
			index = shape->eleBuf[0][i + 2];
			GLfloat Cx = shape->posBuf->at(3 * index);
			GLfloat Cy = shape->posBuf->at(3 * index + 1);
			GLfloat Cz = shape->posBuf->at(3 * index + 2);
			vec3 C(Cx, Cy, Cz);

			/* create new vec3s representing B - A, and C - A */
			vec3 U = B - A;
			vec3 V = C - A;
			/* cross product these two vectors */
			vec3 nor = cross(U, V);
			
			/* fill the normal buffer - x, y, z for three points */
			for (int j = 0; j < 3; j++)
			{
				alduinNorFlat[3 * i + 3 * j] = nor.x;
				alduinNorFlat[3 * i + 3 * j + 1] = nor.y;
				alduinNorFlat[3 * i + 3 * j + 2] = nor.z;
			}
		}

		/* copy data from our nor buffer to VBO*/
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat) * shape->eleBuf[0].size(),
			alduinNorFlat, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		/* create VBO for smoothed normals
		 * Approach: run through the element buffer,
		 * collect indices of eleBuf that refer to the same vertex,
		 * and use those indices to average normals */

		/* keep a 2D vector: its index represents the index of the vertex
		 * it contains vectors that will contain eleBuf indices
		 * referring to that vertex index */
		vector<vector<unsigned int>> sharedEles(shape->posBuf->size() / 3,
			vector<unsigned int>());

		/* gather all the common indices in the element buffer*/
		for (i = 0; i < shape->eleBuf[0].size(); i++)
		{
			sharedEles[shape->eleBuf[0][i]].push_back(i);
		}

		GLfloat *alduinNorSmooth = new GLfloat[3 * shape->eleBuf[0].size()];

		/* iterate through each vertex, averaging it if it has multiple normals */
		for (i = 0; i < sharedEles.size(); i++)
		{
			int j;
			float x_sum = 0.0f, y_sum = 0.0f, z_sum = 0.0f;
			vector<unsigned int> elesToAvg = sharedEles[i];
			for (j = 0; j < elesToAvg.size(); j++)
			{
				x_sum += alduinNorFlat[3 * elesToAvg[j]];
				y_sum += alduinNorFlat[3 * elesToAvg[j] + 1];
				z_sum += alduinNorFlat[3 * elesToAvg[j] + 2];
			}
			float x = x_sum / (float)elesToAvg.size();
			float y = y_sum / (float)elesToAvg.size();
			float z = z_sum / (float)elesToAvg.size();

			for (j = 0; j < elesToAvg.size(); j++)
			{
				alduinNorSmooth[3 * elesToAvg[j]] = x;
				alduinNorSmooth[3 * elesToAvg[j] + 1] = y;
				alduinNorSmooth[3 * elesToAvg[j] + 2] = z;
			}
		}

		/* generate VBO for smooth normals */
		glGenBuffers(1, &VertexNorIDSmooth);
		glBindBuffer(GL_ARRAY_BUFFER, VertexNorIDSmooth);

		/* copy data from our nor buffer to VBO*/
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat) * shape->eleBuf[0].size(),
			alduinNorSmooth, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glBindVertexArray(0);
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.2f, 0.5f, 0.75f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl", resourceDirectory + "/shader_geometry.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addUniform("alduinNum");
		prog->addUniform("time");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNorF");
		prog->addAttribute("vertTex");
		prog->addAttribute("vertNorS");
	}


	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		double frametime = get_last_elapsed_time();

		// Get current frame buffer size.
		int width, height;
		int dragNo = 0;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = glm::mat4(1);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);		
		if (width < height)
			{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect,  1.0f / aspect, -2.0f, 100.0f);
			}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones

		//animation with the model matrix:
		static float w = 0.0, ww = 0.0;
		w += 1.0 * frametime; //rotation angle
		ww -= 1.0 * frametime;
		float trans = 0;// sin(t) * 2;
		glm::mat4 RotateYA = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0.0f, 1.0f, 0.0f));
		float angle = -3.1415926/2.0;
		glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3 + trans));
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		glm::mat4 TransX_A = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f));

		M = TransX_A * TransZ * RotateYA * RotateX * S;

		// Draw the box using GLSL.
		prog->bind();

		V = mycam.process(frametime);

		glBindVertexArray(VertexArrayID);

		//send the matrices to the shaders
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
		glUniform1i(prog->getUniform("alduinNum"), dragNo);
		glUniform1f(prog->getUniform("time"), glfwGetTime());

		glDrawArrays(GL_TRIANGLES, 0, 3 * shape->eleBuf[0].size());

		glm::mat4 TransX_B = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 RotateYB = glm::rotate(glm::mat4(1.0f), ww, glm::vec3(0.0f, 1.0f, 0.0f));
		M = TransX_B * TransZ * RotateYB * RotateX * S;
		dragNo += 1;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform1i(prog->getUniform("alduinNum"), dragNo);

		glDrawArrays(GL_TRIANGLES, 0, 3 * shape->eleBuf[0].size());

		glBindVertexArray(0);

		prog->unbind();

	}

	/* Modification of render function for VR headset */

	void render_vr(int width, int height, glm::mat4 VRheadmatrix)
	{

		double frametime = get_last_elapsed_time();
		int dragNo = 0;

		// Get current frame buffer size.
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now

		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = VRheadmatrix;

		M = glm::mat4(1);
	
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones

		//animation with the model matrix:
		static float w = 0.0, ww = 0.0;
		w += 1.0 * frametime; //rotation angle
		ww -= 1.0 * frametime;
		float trans = 0;// sin(t) * 2;
		glm::mat4 RotateYA = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0.0f, 1.0f, 0.0f));
		float angle = -3.1415926 / 2.0;
		glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3 + trans));
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		glm::mat4 TransX_A = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f));

		M = TransX_A * TransZ * RotateYA * RotateX * S;

		// Draw the box using GLSL.
		prog->bind();

		V = mycam.process(frametime) * VRheadmatrix;

		glBindVertexArray(VertexArrayID);

		//send the matrices to the shaders
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
		glUniform1i(prog->getUniform("alduinNum"), dragNo);
		glUniform1f(prog->getUniform("time"), glfwGetTime());

		glDrawArrays(GL_TRIANGLES, 0, 3 * shape->eleBuf[0].size());

		glm::mat4 TransX_B = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 RotateYB = glm::rotate(glm::mat4(1.0f), ww, glm::vec3(0.0f, 1.0f, 0.0f));
		M = TransX_B * TransZ * RotateYB * RotateX * S;
		dragNo += 1;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform1i(prog->getUniform("alduinNum"), dragNo);

		glDrawArrays(GL_TRIANGLES, 0, 3 * shape->eleBuf[0].size());

		glBindVertexArray(0);

		prog->unbind();
	}

};

Application *application = new Application();

void renderfct(int w, int h, glm::mat4 VRheadmatrix)
{
	application->render_vr(w, h, VRheadmatrix);
}
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	OpenVRApplication *vrapp = NULL;

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	vrapp = new OpenVRApplication;

	windowManager->init(vrapp->get_render_width(), vrapp->get_render_height());
	windowManager->setEventCallbacks(application);

	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	vrapp->init_buffers(resourceDir); //resourceDir .. where the GLSL shader files can be found

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		vrapp->render_to_VR(renderfct);
	    vrapp->render_to_screen(0);//0..left eye, 1..right eye

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
