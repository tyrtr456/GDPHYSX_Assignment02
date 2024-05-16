#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <string>
#include <iostream>

float translate_x_mod = 0.f;
float translate_y_mod = 0.f;
float rotate_x_mod = 0.f;
float rotate_y_mod = 1.f;
float scale_mod = 1.f;
float zoom_mod = -5.f;
int activeModelIndex = 0;

class Model {
public:
    Model(const std::string& path) {
        loadModel(path);
        initializeBuffers();
    }

    void setPosition(float posX, float posY, float posZ) {
        position = glm::vec3(posX, posY, posZ);
    }

    void setRotation(float rotX, float rotY, float rotZ) {
        rotation = glm::vec3(rotX, rotY, rotZ);
    }

    void setScale(float scaleX, float scaleY, float scaleZ) {
        scale = glm::vec3(scaleX, scaleY, scaleZ);
    }

    void initializeVertexData() {
        for (int i = 0; i < shapes[0].mesh.indices.size(); i++) {
            tinyobj::index_t vData = shapes[0].mesh.indices[i];

            // Push x, y, z positions
            fullVertexData.push_back(attributes.vertices[vData.vertex_index * 3]);
            fullVertexData.push_back(attributes.vertices[vData.vertex_index * 3 + 1]);
            fullVertexData.push_back(attributes.vertices[vData.vertex_index * 3 + 2]);

            // Push x, y, z normals
            fullVertexData.push_back(attributes.normals[vData.normal_index * 3]);
            fullVertexData.push_back(attributes.normals[vData.normal_index * 3 + 1]);
            fullVertexData.push_back(attributes.normals[vData.normal_index * 3 + 2]);

            // Push u, v texture coordinates
            fullVertexData.push_back(attributes.texcoords[vData.texcoord_index * 2]);
            fullVertexData.push_back(attributes.texcoords[vData.texcoord_index * 2 + 1]);

            // Push tangents and bitangents
            fullVertexData.push_back(tangents[i].x);
            fullVertexData.push_back(tangents[i].y);
            fullVertexData.push_back(tangents[i].z);
            fullVertexData.push_back(bitangents[i].x);
            fullVertexData.push_back(bitangents[i].y);
            fullVertexData.push_back(bitangents[i].z);
        }
    }

    void draw() {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, fullVertexData.size() / 14);
        glBindVertexArray(0);
    }


private:

    void loadModel(const std::string& path) {
        std::string warning, error;
        bool success = tinyobj::LoadObj(
            &attributes,
            &shapes,
            &material,
            &warning,
            &error,
            path.c_str()
        );

        if (!success) {
            std::cerr << "Error loading model: " << error << std::endl;
            return;
        }
    }

    void extractTangentsAndBitangents() {

        for (int i = 0; i < shapes[0].mesh.indices.size(); i += 3) {
            //get vertex data for triangle
            tinyobj::index_t vData1 = shapes[0].mesh.indices[i];
            tinyobj::index_t vData2 = shapes[0].mesh.indices[i + 1];
            tinyobj::index_t vData3 = shapes[0].mesh.indices[i + 2];

            //position of vertex 1
            glm::vec3 v1 = glm::vec3(
                attributes.vertices[vData1.vertex_index * 3],
                attributes.vertices[(vData1.vertex_index * 3) + 1],
                attributes.vertices[(vData1.vertex_index * 3) + 2]
            );

            //position of vertex 2
            glm::vec3 v2 = glm::vec3(
                attributes.vertices[vData2.vertex_index * 3],
                attributes.vertices[(vData2.vertex_index * 3) + 1],
                attributes.vertices[(vData2.vertex_index * 3) + 2]
            );

            //position of vertex 3
            glm::vec3 v3 = glm::vec3(
                attributes.vertices[vData3.vertex_index * 3],
                attributes.vertices[(vData3.vertex_index * 3) + 1],
                attributes.vertices[(vData3.vertex_index * 3) + 2]
            );

            //uv of vertex 1
            glm::vec2 uv1 = glm::vec2(
                attributes.texcoords[(vData1.texcoord_index * 2)],
                attributes.texcoords[(vData1.texcoord_index * 2) + 1]
            );

            //uv of vertex 2
            glm::vec2 uv2 = glm::vec2(
                attributes.texcoords[(vData2.texcoord_index * 2)],
                attributes.texcoords[(vData2.texcoord_index * 2) + 1]
            );

            //uv of vertex 3
            glm::vec2 uv3 = glm::vec2(
                attributes.texcoords[(vData3.texcoord_index * 2)],
                attributes.texcoords[(vData3.texcoord_index * 2) + 1]
            );

            //edges of triangle: position delta
            glm::vec3 deltaPos1 = v2 - v1;
            glm::vec3 deltaPos2 = v3 - v1;

            //uv delta
            glm::vec2 deltaUV1 = uv2 - uv1;
            glm::vec2 deltaUV2 = uv3 - uv1;

            float r = 1.0f / ((deltaUV1.x * deltaUV2.y) - (deltaUV1.y * deltaUV2.x));
            //tangent (T)
            glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
            //bitangent (B)
            glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

            //push back tangent and bitangent three times
            tangents.push_back(tangent);
            tangents.push_back(tangent);
            tangents.push_back(tangent);

            bitangents.push_back(bitangent);
            bitangents.push_back(bitangent);
            bitangents.push_back(bitangent);
        }
    }

    void initializeBuffers() {
        // Call extractTangentsAndBitangents to populate tangents and bitangents
        extractTangentsAndBitangents();
        // Initialize vertex data
        initializeVertexData();
        // Generate VAO and VBO IDs
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        // Bind the VAO
        glBindVertexArray(VAO);

        // Bind the VBO and send vertex data to the GPU
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * fullVertexData.size(), fullVertexData.data(), GL_DYNAMIC_DRAW);

        // Specify vertex attributes layout
        glVertexAttribPointer(
            0, //vertex position
            3, // x y z
            GL_FLOAT, //data type of array
            GL_FALSE,
            //vertex data has 8 floats
            //X,Y,Z,3 normals, U,V
            14 * sizeof(float), //size of vertex data in bytes
            (void*)0
        );
        //enable vertices, this time for skybox
        glEnableVertexAttribArray(0);

        //normal starts at index 3
        GLintptr normalPtr = 3 * sizeof(float);

        glVertexAttribPointer(
            1, //normals
            3, // x y z
            GL_FLOAT, //data type of array
            GL_FALSE,
            //vertex data has 8 floats
            //X,Y,Z,3 normals, U,V
            14 * sizeof(float), //size of vertex data in bytes
            (void*)normalPtr
        );
        //enable normals
        glEnableVertexAttribArray(1);

        //uv starts at index 6
        GLintptr uvPtr = 6 * sizeof(float);

        //get uv data from array
        glVertexAttribPointer(
            2, //uv coords
            2, // u v
            GL_FLOAT, //data type of array
            GL_FALSE,
            //vertex data has 8 floats
            //X,Y,Z,3 normals, U,V
            14 * sizeof(float), //size of vertex data in bytes
            //offset
            (void*)uvPtr
        );

        //enable textures
        glEnableVertexAttribArray(2);

        //tangent starts at index 8
        GLintptr tangentPtr = 8 * sizeof(float);

        glVertexAttribPointer(
            3, //tangent
            3, // T (xyz)
            GL_FLOAT, //data type of array
            GL_FALSE,
            14 * sizeof(float), //size of vertex data in bytes
            //offset
            (void*)tangentPtr
        );
        //enable tangent
        glEnableVertexAttribArray(3);

        //bitangent starts at index 11
        GLintptr bitangentPtr = 11 * sizeof(float);

        glVertexAttribPointer(
            4, //bitangent
            3, // T (xyz)
            GL_FLOAT, //data type of array
            GL_FALSE,
            14 * sizeof(float), //size of vertex data in bytes
            //offset
            (void*)bitangentPtr
        );
        //enable bitangent
        glEnableVertexAttribArray(4);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Unbind the VBO and VAO
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }


    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> material;
    tinyobj::attrib_t attributes;
    std::vector<GLfloat> fullVertexData;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    GLuint VAO, VBO;
    
};

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath) {
        vertexCode = readFile(vertexPath);
        fragmentCode = readFile(fragmentPath);

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        // Compile shaders
        GLuint vertex, fragment;
        GLint success;
        GLchar infoLog[512];

        // Vertex Shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cout << "Vertex shader compilation failed:\n" << infoLog << std::endl;
        }

        // Fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragment, 512, NULL, infoLog);
            std::cout << "Fragment shader compilation failed:\n" << infoLog << std::endl;
        }

        // Shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(ID, 512, NULL, infoLog);
            std::cout << "Shader program linking failed:\n" << infoLog << std::endl;
        }

        // Delete the shaders as they're linked into our program now and no longer necessary
        //glDeleteShader(vertex);
        //glDeleteShader(fragment);
    }

    // Use the shader
    void use() {
        glUseProgram(ID);
    }

    // Set projection matrix uniform
    void setProjectionMatrix(const glm::mat4& projectionMatrix) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    }

    // Set view matrix uniform
    void setViewMatrix(const glm::mat4& viewMatrix) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    }

    // Set transform matrix uniform
    void setTransformMatrix(const glm::mat4& transformation_matrix) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, "transform"), 1, GL_FALSE, glm::value_ptr(transformation_matrix));
    }

    // Set texture uniforms
    void setTextureUniforms(GLuint texture, GLuint norm_tex) const {
        glUniform1i(glGetUniformLocation(ID, "tex0"), 0);
        glUniform1i(glGetUniformLocation(ID, "norm_tex"), 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, norm_tex);
    }

    // Set lighting uniforms
    void setLightingUniforms(const glm::vec3& lightPos, const glm::vec3& lightColor,
        float ambientStr, const glm::vec3& ambientColor,
        const glm::vec3& cameraPos, float specStr, float specPhong) const {
        glUniform3fv(glGetUniformLocation(ID, "lightPos"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(ID, "lightColor"), 1, glm::value_ptr(lightColor));
        glUniform1f(glGetUniformLocation(ID, "ambientStr"), ambientStr);
        glUniform3fv(glGetUniformLocation(ID, "ambientColor"), 1, glm::value_ptr(ambientColor));
        glUniform3fv(glGetUniformLocation(ID, "cameraPos"), 1, glm::value_ptr(cameraPos));
        glUniform1f(glGetUniformLocation(ID, "specStr"), specStr);
        glUniform1f(glGetUniformLocation(ID, "specPhong"), specPhong);
    }

private:
    std::string vertexCode;
    std::string fragmentCode;
    GLuint ID;

    std::string readFile(const std::string& path) {
        std::ifstream file(path);
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        return stream.str();
    }
};

class Camera {
public:
    Camera(GLFWwindow* window) : window(window), position(glm::vec3(0.0f, 0.0f, 3.0f)), front(glm::vec3(0.0f, 0.0f, -1.0f)), up(glm::vec3(0.0f, 1.0f, 0.0f)), yaw(-90.0f), pitch(0.0f), movementSpeed(2.5f), mouseSensitivity(0.1f), zoom(45.0f) {
        this->updateCameraVectors();
    }

    void processInput(float deltaTime) {
        float velocity = movementSpeed * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            position += front * velocity;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            position -= front * velocity;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            position -= glm::normalize(glm::cross(front, up)) * velocity;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            position += glm::normalize(glm::cross(front, up)) * velocity;
    }

    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->front = glm::normalize(front);
        right = glm::normalize(glm::cross(front, up));
        this->up = glm::normalize(glm::cross(right, front));
    }

    glm::mat4 getViewMatrix() {
        return glm::lookAt(position, position + front, up);
    }

    float getZoom() const {
        return zoom;
    }

private:
    GLFWwindow* window;
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;
    float zoom;
};

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    float window_width = 800.0f;
    float window_height = 800.0f;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(window_width, window_height, "Pascual, Uriel", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    gladLoadGL();

    int img_width, //texture width
        img_height, //texture height
        colorChannels; //number of collor channels

    //flip texture
    stbi_set_flip_vertically_on_load(true);
    //load texture
    unsigned char* tex_bytes = stbi_load("3D/brickwall.jpg", //texture path
        &img_width, //fill out width
        &img_height, //fill out height
        &colorChannels, //fill out colors
        0);

    //opengl reference to textures
    GLuint texture;
    //generate reference
    glGenTextures(1, &texture);
    //set current texture working on
    glActiveTexture(GL_TEXTURE0);
    //bind texture
    glBindTexture(GL_TEXTURE_2D, texture);


    //assing texture to opengl reference
    glTexImage2D(GL_TEXTURE_2D,
        0, //texture 0
        GL_RGB, //target color format
        img_width, //texture width
        img_height, //texture height
        0,
        GL_RGB, //color format of texture
        GL_UNSIGNED_BYTE,
        tex_bytes); //load texture in bytes

    //generate mipmap
    glGenerateMipmap(GL_TEXTURE_2D);
    //free up loaded bytes
    stbi_image_free(tex_bytes);

    //below are loading normals
    int img_width2, img_height2, colorChannel2;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* normal_bytes = stbi_load("3D/brickwall_normal.jpg",
        &img_width2,
        &img_height2,
        &colorChannel2,
        0);

    //opengl reference to normal textures
    GLuint norm_tex;
    //generate normal reference
    glGenTextures(1, &norm_tex);
    //set normals since texture 0 is used alrready
    glActiveTexture(GL_TEXTURE1);
    //bind normal texture
    glBindTexture(GL_TEXTURE_2D, norm_tex);
    //set parameters to be the same as diffuse color
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    //assing normal texture to opengl reference
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        img_width2,
        img_height2,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        normal_bytes);

    //generate mipmap of normal map
    glGenerateMipmap(GL_TEXTURE_2D);
    //free up loaded bytes
    stbi_image_free(normal_bytes);
   
    //filtering cubemap
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //stretch to edge
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //re-enable image flip
    stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);

    Camera camera(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, &camera);

    Shader shader("Shaders/sample.vert", "Shaders/sample.frag");
    shader.use();

 

    GLint isCompiled = 0;

    GLuint VAO, VBO;
    //Initialize VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    //enable vertices
    glEnableVertexAttribArray(0);

    float x, y, z;
    x = y = z = 0.0f;
    z = -20.0f;
    glm::mat3 identity_matrix3 = glm::mat3(1.0f);
    glm::mat4 identity_matrix4 = glm::mat4(1.0f);

    float scale_x, scale_y, scale_z = 1.f;
    scale_x = scale_y = scale_z = 1.f;

    float axis_x, axis_y, axis_z, theta = 0.f;
    axis_x = axis_y = axis_z = theta = 0.f;
    axis_y = 1.f;

    /*glm::mat4 projectionMatrix = glm::ortho(
        -1.f, //left
        1.f, //right
        -1.f, //bottom
        1.f, //top
        -1.f, //znear
        1.f //zfar
    );*/

    glm::mat4 projectionMatrix = glm::ortho(
        -window_width / 2.0f, // left
        window_width / 2.0f,  // right
        -window_height / 2.0f, // bottom
        window_height / 2.0f,  // top
        0.1f,  // znear
        175.0f // zfar
    );

    // Set camera position
    glm::vec3 cameraPos = glm::vec3(rotate_x_mod, 0, 100.f);

    glm::mat4 cameraPositionMatrix = glm::translate(glm::mat4(1.0f),
        cameraPos * -1.0f);
    // World up
    glm::vec3 WorldUp = glm::vec3(0, 1.0f, 0);
    // Camera center
    glm::vec3 Center = glm::vec3(0, 3.f, 0);

    glm::vec3 F = glm::vec3(Center - cameraPos);
    F = glm::normalize(F);

    glm::vec3 R = glm::normalize(glm::cross(F, WorldUp));

    glm::vec3 U = glm::normalize(glm::cross(R, F));

    // Camera orientation matrix
    glm::mat4 cameraOrientation = glm::mat4(1.f);

    // Assign matrix
    // Matrix[Column][Row]
    cameraOrientation[0][0] = R.x;
    cameraOrientation[1][0] = R.y;
    cameraOrientation[2][0] = R.z;

    cameraOrientation[0][1] = U.x;
    cameraOrientation[1][1] = U.y;
    cameraOrientation[2][1] = U.z;

    cameraOrientation[0][2] = -F.x;
    cameraOrientation[1][2] = -F.y;
    cameraOrientation[2][2] = -F.z;

    glm::mat4 viewMatrix = cameraOrientation * cameraPositionMatrix;

    //light position
    glm::vec3 lightPos = glm::vec3(0, 0, 0);
    //diff light color
    glm::vec3 lightColor = glm::vec3(2, 2, 2);
    //ambient strength
    float ambientStr = 0.1f;
    //ambient color
    glm::vec3 ambientColor = lightColor;
    //spec strength
    float specStr = 0.5f;
    //spec phong
    float specPhong = 16;

    //enable blending
    glEnable(GL_BLEND);

    //blending function
    glBlendFunc(GL_SRC_ALPHA, //source factor
        GL_ONE_MINUS_SRC_ALPHA //destination factor
    );

    glBlendEquation(GL_FUNC_ADD);

    Model sphere("3D/sphere.obj");
    Model brickwall("3D/plane.obj");

    // Set position, rotation, and scale
    sphere.setPosition(0.0f, 0.0f, 0.0f);
    sphere.setRotation(0.0f, 0.0f, 0.0f);
    sphere.setScale(1.0f, 1.0f, 1.0f);

    brickwall.setPosition(-10.0f, 3.0f, 0.0f);
    brickwall.setRotation(0.0f, 0.0f, 0.0f);
    brickwall.setScale(1.0f, 1.0f, 1.0f);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {  
        /* Poll for and process events */
        glfwPollEvents();

        float x, y, z;
        x = y = z = 0.0f;
        z = -5.0f; // Adjust this value to move the object closer

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
        glm::mat4 view = camera.getViewMatrix();
        shader.setProjectionMatrix(projection);
        shader.setViewMatrix(view);

        glm::mat4 model = glm::mat4(1.0f);
        //camera.processInput(0.1f);

        glm::mat4 transformation_matrix = glm::translate(
            identity_matrix4,
            glm::vec3(x, y, z)
        );

        transformation_matrix = glm::scale(
            transformation_matrix,
            glm::vec3(scale_x + scale_mod, scale_y + scale_mod, scale_z)
        );

        transformation_matrix = glm::rotate(
            transformation_matrix,
            glm::radians(theta),
            glm::normalize(glm::vec3(axis_x, axis_y, axis_z))
        );

        transformation_matrix = glm::rotate(
            transformation_matrix,
            glm::radians(axis_y),
            glm::normalize(glm::vec3(1.0f, 0.0f, axis_z))
        );

        shader.setTransformMatrix(model);
        shader.setProjectionMatrix(projectionMatrix);
        shader.setViewMatrix(viewMatrix);

        //disable mask
        glDepthMask(GL_FALSE);
        //change depth function into <=
        glDepthFunc(GL_LEQUAL);
        //reset depth to true
        glDepthMask(GL_TRUE);
        //reset depth to normal
        glDepthFunc(GL_LESS);

        //draw other stuff below
        shader.use();

        glBindVertexArray(VAO);

        shader.setTransformMatrix(transformation_matrix);
        shader.setTextureUniforms(texture, norm_tex);
        shader.setLightingUniforms(lightPos, lightColor, ambientStr, ambientColor, cameraPos, specStr, specPhong);

        

        //glDrawArrays(GL_TRIANGLES, 0, fullVertexData.size() / 14);
        // Draw the model
        sphere.draw();
        brickwall.draw();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}