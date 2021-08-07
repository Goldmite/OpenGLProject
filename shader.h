#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // include glad to get all the required OpenGL headers

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class GeneralShader {
public:
    unsigned int ID;
    //read and build shader
    GeneralShader(const char* VertexPath, const char* FragmentPath)
    {
        //retrieve the vertex/fragment source code from file path
        std::string VertexCode;
        std::string FragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        //ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try {
            //open files
            vShaderFile.open(VertexPath);
            fShaderFile.open(FragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            //read files buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            //close files
            vShaderFile.close();
            fShaderFile.close();
            //convert stream to string
            VertexCode = vShaderStream.str();
            FragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "Shader file not succesfully read" << std::endl;
        }
        const char* vShaderCode = VertexCode.c_str();
        const char* fShaderCode = FragmentCode.c_str();
        //compile shaders
        unsigned int vertex, fragment;
        //vertex shaders
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        //fragment shaders
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        //program shaders
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        //delete unnecessary shaders
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    //use shader
    void use(){
        glUseProgram(ID);
    }
    //utility uniform funtions
    void setBool(const std::string &name, bool value) const{
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }
    void setInt(const std::string &name, int value) const{
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    void setFloat(const std::string &name, float value) const{
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    }
private:
    void checkCompileErrors(unsigned int shader, std::string type){
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "Shader failed to compile. Error of type: " << type << "\n" << infoLog << "\n" << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "Program failed to link. Error of type: " << type << "\n" << infoLog << "\n" << std::endl;
            }
        } 
    }
};
class ComputeShader {
public:
    unsigned int ID;
    //read and build shader
    ComputeShader(const char* ComputePath)
    {
        //retrieve the vertex/fragment source code from file path
        std::string ComputeCode;
        std::ifstream cShaderFile;
        //ensure ifstream objects can throw exceptions:
        cShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try {
            //open files
            cShaderFile.open(ComputePath);
            std::stringstream cShaderStream;
            //read files buffer contents into streams
            cShaderStream << cShaderFile.rdbuf();
            //close files
            cShaderFile.close();
            //convert stream to string
            ComputeCode = cShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "Compute shader file unsuccesfully read" << std::endl;
        }
        const char* cComputeCode = ComputeCode.c_str();
        //compile shaders
        unsigned int compute;
        //compute shaders
        compute = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(compute, 1, &cComputeCode, NULL);
        glCompileShader(compute);
        checkCompileErrors(compute, "COMPUTE");
        //program shaders
        ID = glCreateProgram();
        glAttachShader(ID, compute);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        glDeleteShader(compute);
    }
    //use shader
    void use(){
        glUseProgram(ID);
    }
    void dispatch(int x, int y){
        glDispatchCompute(x, y, 1);
    }
    //utility uniform funtions
    void setBool(const std::string &name, bool value) const{
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }
    void setInt(const std::string &name, int value) const{
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    void setFloat(const std::string &name, float value) const{
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    }
private:
    void checkCompileErrors(unsigned int shader, std::string type){
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "Compute shader failed to compile. Error of type: " << type << "\n" << infoLog << "\n" << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "Compute Program failed to link. Error of type: " << type << "\n" << infoLog << "\n" << std::endl;
            }
        } 
    }
};
#endif