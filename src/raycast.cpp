#define GLM_FORCE_PURE

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi
#include <iostream>
#include <cmath>
#include <vector>
#include <cstring>

#include <common/controls.hpp>
#include <common/shader.hpp>

#include <engine/scene.hpp>
#include <engine/object.hpp>
#include <engine/canvas.hpp>
#include <engine/collision.hpp>


#define M_PI        3.14159265358979323846
#define onii        using
#define oniichan    using
#define suki        std::cout
#define hentai      std::endl
#define chan        namespace
#define mendokusai  std::chrono
#define daikirai    glm

#define gohan_ni     using
#define ofuro_ni     using
// #define ni          namespace
#define suru         glm::vec3
#define suru_        glm::mat4
#define soretomo     using
#define watashi      glm::vec4

// tehee
oniichan suki;
oniichan hentai;
onii chan mendokusai;
onii chan daikirai;

gohan_ni suru;//?
ofuro_ni suru_;//?
soretomo watashi;//?

enum class CAMERA_MODE{
    PROJECTION_1,
    PROJECTION_2,
    PROJECTION_3,
    LIGHT,
    FREE_VIEW,
};

class MainScene : engine::Scene {
public:
    GLFWwindow* window;

    std::vector<engine::Object*> objects;
    engine::Object *barrel_1, *barrel_2, *barrel_3, *plane;
    engine::Canvas *canvas;

    CAMERA_MODE camera_mode = CAMERA_MODE::PROJECTION_1;

    GLuint shader_depth, shader;
    double timer = 1;
    double frame_time;
    int frame_count = 0;

    MainScene (GLFWwindow* window): Scene(window) {
        this->window = window;

        glClearColor(0.5f, 0.5f, 0.5f, 1.f);

        // Enable depth test
        glEnable(GL_DEPTH_TEST);

        // Accept fragment if it is closer to the camera than the former one
        // glDepthFunc(GL_LEQUAL); 

        // Cull triangles which normal is not towards the camera
        // glEnable(GL_CULL_FACE);

        // Blending
        glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Associate the object instance with the GLFW window
        glfwSetWindowUserPointer(window, this);

        start();
    }

    void start() override {
        // Light
        LightPosition = vec3(0, 10, 0);
        

        // create shaders
        shader = LoadShaders("res/shader/Textured_With_Light.vs", "res/shader/Textured_With_Light.fs");

        // ======================== PLANE ========================

        // LOAD PLANE MODEL
        plane = new engine::Object("res/obj/plane.obj", "res/bmp/plane.bmp", shader, this);
        

        // ======================== BARREL ========================

        // LOAD BARREL MODEL
        barrel_1 = new engine::Object("res/obj/barrel.obj", "res/bmp/barrel.bmp", shader, this);
        barrel_1->transform = glm::translate(barrel_1->transform, vec3(4, 1, 1));
        

        barrel_2 = new engine::Object("res/obj/barrel.obj", "res/bmp/barrel.bmp", shader, this);
        barrel_2->transform = glm::translate(barrel_2->transform, vec3(8, 1, -7));
        

        
        barrel_3 = new engine::Object("res/obj/barrel.obj", "res/bmp/barrel.bmp", shader, this);
        barrel_3->transform = glm::translate(barrel_3->transform, vec3(16, 1, -13));
        
        // Store in the container (objects group)
        objects.push_back(barrel_1);
        objects.push_back(barrel_2);
        objects.push_back(barrel_3);

        // ======================== Configure depth map FBO ========================

        // binding keys
        glfwSetKeyCallback(window, keyCallbackStatic);
        glfwSetMouseButtonCallback(window, mouseCallbackStatic);
    }



    void update() override {
        Scene::update();

        // calculating FPS
        timer -= delta_time/10e8;
        frame_count++;
        if (timer < 0) {
            std::cout << "FPS: " << frame_count << std::endl;
            timer = 1;
            frame_count = 0;
        }

        // Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // moving light in circular pattern
        LightPosition.x = 0;
        LightPosition.z = 5.f;
        LightPosition.y = 10.f;

        // ======================== CAMERA STUFF ========================

        // vec3 camera_position;
        auto up = vec3( 0, 1, 0);
        auto aspect_ratio = (float)window_width/window_height;
        switch (camera_mode) {
            case CAMERA_MODE::PROJECTION_1:
                ViewMatrix = glm::lookAt(
                    vec3(0, 8, 10), // posisi kamera
                    vec3(4, 4, 1), // melihat barrel
                    up
                );
                ProjectionMatrix = glm::perspective<float>(glm::radians(45.f), aspect_ratio, 0.1f, 1000.0f);
                break;  
            case CAMERA_MODE::PROJECTION_2:
                ViewMatrix = glm::lookAt(
                    vec3(0, 1, 0), // posisi kamera
                    vec3(4, 1, 1), // melihat barrel
                    up
                );
                ProjectionMatrix = glm::perspective<float>(glm::radians(45.f), aspect_ratio, 0.1f, 1000.0f);
                break;
            case CAMERA_MODE::PROJECTION_3:
                ViewMatrix = glm::lookAt(
                    vec3(3, 2, 6), // posisi kamera
                    vec3(4, 1, 1), // melihat barrel
                    up
                );
                ProjectionMatrix = glm::perspective<float>(glm::radians(45.f), aspect_ratio, 0.1f, 1000.0f);
                break;
            case CAMERA_MODE::LIGHT:
                ViewMatrix = glm::lookAt(
                    LightPosition, // posisi kamera
                    vec3(4, 2, 1), // melihat barrel
                    up
                );
                ProjectionMatrix = glm::perspective<float>(glm::radians(45.f), aspect_ratio, 0.1f, 1000.0f);
                break;
            case CAMERA_MODE::FREE_VIEW:
                // Don't touch
                computeMatricesFromInputs(window);
                ViewMatrix = getViewMatrix();
                ProjectionMatrix = getProjectionMatrix();
                break;
        }

        auto before_transform = barrel_1->transform;

        if(glfwGetKey(window,  GLFW_KEY_A) == GLFW_PRESS){
            barrel_1->transform = glm::translate(barrel_1->transform, vec3(1 * delta_time/1e8, 0, 0));
        }
        if(glfwGetKey(window,  GLFW_KEY_D) == GLFW_PRESS){
            barrel_1->transform = glm::translate(barrel_1->transform, vec3(-1 * delta_time/1e8, 0, 0));
        }
        if(glfwGetKey(window,  GLFW_KEY_W) == GLFW_PRESS){
            barrel_1->transform = glm::translate(barrel_1->transform, vec3(0, 0, 1 * delta_time/1e8));
        }
        if(glfwGetKey(window,  GLFW_KEY_S) == GLFW_PRESS){
            barrel_1->transform = glm::translate(barrel_1->transform, vec3(0, 0, -1 * delta_time/1e8));
        }

        if (is_colliding(barrel_1, barrel_2) ||  is_colliding(barrel_1, barrel_3)) {
            barrel_1->transform = before_transform;
        }

        

        // ======================== RENDER SCENE =======================
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(shader);
        
        plane->render_with_projection();
        barrel_1->render_with_projection();
        barrel_2->render_with_projection();
        barrel_3->render_with_projection();
    }

private:
    bool is_colliding(engine::Object *obj1, engine::Object *obj2) {
        auto obj1_min = obj1->transform * obj1->bounding_planes[0];
        auto obj2_min = obj2->transform * obj2->bounding_planes[0];

        auto obj1_max = obj1->transform * obj1->bounding_planes[1];
        auto obj2_max = obj2->transform * obj2->bounding_planes[1];

        // Check collision along X axis
        if (obj1_max.x < obj2_min.x || obj1_min.x > obj2_max.x)
            return false;

        // Check collision along Y axis
        if (obj1_max.y < obj2_min.y || obj1_min.y > obj2_max.y)
            return false;

        // Check collision along Z axis
        if (obj1_max.z < obj2_min.z || obj1_min.z > obj2_max.z)
            return false;
        
        return true;
    };
    
    engine::Ray castRayFromScreen(int mouseX, int mouseY, int screenWidth, int screenHeight, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
        // Convert screen coordinates to NDC
        float ndcX = (2.0f * mouseX) / screenWidth - 1.0f;
        float ndcY = 1.0f - (2.0f * mouseY) / screenHeight;

        // Combine into vec4 (x, y, depth, w)
        glm::vec4 viewportCoords = glm::vec4(ndcX, ndcY, 0.0f, 1.0f);

        // Unproject NDC to view space
        glm::mat4 inverseVP = glm::inverse(projectionMatrix * viewMatrix);
        glm::vec4 viewCoords = inverseVP * viewportCoords;
        viewCoords /= viewCoords.w; // Normalize by w component

        // Origin of the ray is the camera position
        glm::vec3 rayOrigin = glm::vec3(viewCoords);

        // Now define the direction of the ray (e.g., towards a point in the scene)
        // hanya work untuk PROJECTION_1 karena melihat langsung ke arah (4,1,1)
        glm::vec3 cameraDirection = glm::vec3(4, 1, 1);

        // Create the ray
        engine::Ray ray;
        ray.origin = rayOrigin;
        ray.direction = cameraDirection;

        return ray;
    }

    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
        if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            double xpos, ypos;
            //getting cursor position
            glfwGetCursorPos(window, &xpos, &ypos);
            cout << "Cursor Position at (" << xpos << " : " << ypos << ")" << endl;

            // Do the raycast
            engine::Ray ray = castRayFromScreen(xpos, ypos, window_width, window_height, ViewMatrix, ProjectionMatrix);
            cout << "Ray Casted: Origin (" << ray.origin.x << " " << ray.origin.y << " " << ray.origin.z << ") " 
                                "Direction (" << ray.direction.x << " " << ray.direction.y << " " << ray.direction.z  << ")" << endl;

        }
    }

    
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        float move_speed = 3.;
        if (action == GLFW_PRESS) {
            std::cout << key << std::endl;
            switch (key) {
            case GLFW_KEY_1:
                camera_mode = CAMERA_MODE::PROJECTION_1;
                break;
            case GLFW_KEY_2:
                camera_mode = CAMERA_MODE::PROJECTION_2;
                break;
            case GLFW_KEY_3:
                camera_mode = CAMERA_MODE::PROJECTION_3;
                break;
            case GLFW_KEY_9:
                camera_mode = CAMERA_MODE::LIGHT;
                break;
            case GLFW_KEY_0:
                camera_mode = CAMERA_MODE::FREE_VIEW;
                break;
            }
        }

        
    }
    static void mouseCallbackStatic(GLFWwindow* window, int button, int action, int mods) {
        MainScene* instance = static_cast<MainScene*>(glfwGetWindowUserPointer(window));
        instance->mouse_button_callback(window, button, action, mods);
    }

    static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods) {
        MainScene* instance = static_cast<MainScene*>(glfwGetWindowUserPointer(window));
        instance->key_callback(window, key, scancode, action, mods);
    }
    

};