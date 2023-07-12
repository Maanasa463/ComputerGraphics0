#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

int toggle = 0;
const float Speed = 0.05f;


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
glm::mat4 view = glm::mat4(1.0f);

glm::vec3 Cam = glm::vec3(0.0f, 0.0f, 10.0f);
glm::vec3 Look = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

int press = 0;

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"

                                 "uniform mat4 model;\n"
                                 "uniform mat4 view;\n"
                                 "uniform mat4 projection;\n"

                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                 "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"

                                   "uniform vec4 ourColor;\n"

                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = ourColor;\n"
                                   "}\n\0";

int main(int argc, char *argv[])
{
    int num_sides = atoi(argv[1]);

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[6 * (num_sides + 1) + 3] = {0};

    vertices[0] = 0.0f;
    vertices[1] = 0.0f;
    vertices[2] = 0.5f;
    vertices[6 * (num_sides + 1) - 2] = -1.0;
    vertices[6 * (num_sides + 1) - 1] = 0.0f;
    vertices[6 * (num_sides + 1)] = 0.0f;


    int j = 1;

    for (int i = 3; i < 3 * (num_sides + 1); i += 3)
    {
        if (j == num_sides / 4 && num_sides % 4 == 0)
            vertices[i] = 0;
        else if (j == 3 * num_sides / 4 && num_sides % 4 == 0)
            vertices[i] = 0;
        else
            vertices[i] = (float)cos(glm::radians((float)j / num_sides * 360.00));

        if (j == num_sides)
            vertices[i + 1] = 0.0f;
        else if (j == num_sides / 2 && num_sides % 2 == 0)
            vertices[i + 1] = 0.0f;
        else
            vertices[i + 1] = (float)sin(glm::radians((float)j / num_sides * 360.00));

        vertices[i + 2] = 0.5f;

        j++;
    }

    // vertices for the back face
    for (int i = 0; i < 3 * (num_sides + 1); i++)
    {
        if (i % 3 == 2)
            vertices[3 * (num_sides + 1) + i] = -vertices[i];
        else
            vertices[3 * (num_sides + 1) + i] = vertices[i];
    }

    unsigned int indices[12 * num_sides + 3 *num_sides] = {0};
    j = 1;

    // loop for the front face
    for (int i = 0; i < 3 * num_sides - 3; i += 3)
    {
        indices[i] = 0;
        indices[i + 1] = j;
        indices[i + 2] = j + 1;

        j++;
    }

    indices[3 * num_sides - 1] = num_sides;
    indices[3 * num_sides - 2] = 1;
    indices[3 * num_sides - 3] = 0;

    // loop for the back face
    for (int i = 0; i < 3 * num_sides; i++)
    {
        indices[3 * num_sides + i] = indices[i] + (num_sides + 1);
    }

    j = 1;

    // loop for the rectangular faces
    for (int i = 0; i < 6 * num_sides; i += 6)
    {
        indices[6 * num_sides + i] = indices[j];
        indices[6 * num_sides + i + 1] = indices[j + 1];
        indices[6 * num_sides + i + 2] = indices[3 * num_sides + j];

        indices[6 * num_sides + i + 3] = indices[j + 1];
        indices[6 * num_sides + i + 4] = indices[3 * num_sides + j];
        indices[6 * num_sides + i + 5] = indices[3 * num_sides + j + 1];

        j += 3;
    }

    j = 1;

    for (int i = 0; i < 3 * num_sides - 3; i += 3)
    {
        indices[12 * num_sides + i] = 2 * num_sides + 2;
        indices[12 * num_sides + i + 1] = j;
        indices[12 * num_sides + i + 2] = j+1;

        j ++;
    }

    indices[15 * num_sides - 1] = 2 * num_sides + 2;
    indices[15 * num_sides - 2] = 1;
    indices[15 * num_sides - 3] = num_sides;

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    glUseProgram(shaderProgram);
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(shaderProgram);

        glm::mat4 model = glm::mat4(1.0f);

        float angle = 45.0f;

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            float X = sin(glfwGetTime()) * 10.0f;
            float Y = cos(glfwGetTime()) * 10.0f;

            view = glm::lookAt(glm::vec3(X, Y, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.5f, 1.0f, 0.5f));
        }
        else if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS
                || glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS
                || glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS
                || glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS
                || glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS
                || glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        {
            view = glm::lookAt(Cam, Cam + Look, Up);
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.5f, 1.0f, 0.5f));

        }
        else
        {
            view = glm::lookAt(Cam, Look, Up);
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.5f, 1.0f, 0.5f));
        }

        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

        if (toggle == 0)
        {

            for (int i = 0; i < 2; i++)
            {
                float greenValue = 0.5f + i;
                int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
                glUseProgram(shaderProgram);

                glUniform4f(vertexColorLocation, 0.0f, greenValue, 1.0f, 1.0f);
                glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
                glDrawElements(GL_TRIANGLES, 3 * (num_sides), GL_UNSIGNED_INT, (void *)(sizeof(float) * (3 * (num_sides)) * i));
            }

            for (int i = 0; i < num_sides; i++)
            {
                float blueValue = 0.05f + ((float)i / num_sides);

                int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
                glUseProgram(shaderProgram);

                glUniform4f(vertexColorLocation, 0.0f, 0.0f, blueValue, 1.0f);
                glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)(sizeof(float) * 6 * ((num_sides + i))));
            }
        }


        if (toggle == 1)
        {
            float greenValue = 0.5f;
            int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
            glUseProgram(shaderProgram);

            glUniform4f(vertexColorLocation, 0.0f, greenValue, 1.0f, 1.0f);
            glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
            glDrawElements(GL_TRIANGLES, 3 * (num_sides), GL_UNSIGNED_INT, (void *)0);
            
            for (int i = 0; i < num_sides; i++)
            {
                float blueValue = 0.05f + ((float)i / num_sides);

                int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
                glUseProgram(shaderProgram);

                glUniform4f(vertexColorLocation, 0.0f, 0.0f, blueValue, 1.0f);
                glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
                glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void *)(sizeof(float) * ((12 * (num_sides)) + (3 * i))));
            }
        }

        glBindVertexArray(0); // no need to unbind it every time

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS )
        Cam += Speed * Look;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        Cam -= Speed * Look;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        Cam -= glm::normalize(glm::cross(Look, Up)) * Speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        Cam += glm::normalize(glm::cross(Look, Up)) * Speed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        Cam += Speed * Up;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        Cam -= Speed * Up;
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        if (toggle == 0)
            toggle = 1;
        else if (toggle == 1)
            toggle = 0;
    }
    if ( glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        Cam += Speed * Look;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        Cam -= Speed * Look;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        Cam -= glm::normalize(glm::cross(Look, Up)) * Speed;
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        Cam += glm::normalize(glm::cross(Look, Up)) * Speed;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        Cam += Speed * Up;
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        Cam -= Speed * Up;
}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}