//
//  main.m
//  OpenGL-公转自转
//
//  Created by zhongding on 2018/12/12.
//


#include "GLShaderManager.h"
#include "StopWatch.h"
#include "GLTools.h"
#include "GLTriangleBatch.h"
#include "GLBatch.h"
#include "GLFrustum.h"
#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"


#ifdef __APPLE__
#include <glut/glut.h>
#else
#include <GL/glut.h>
#endif


GLShaderManager shaderManager;

GLMatrixStack modelMatriStack;
GLMatrixStack projectionMatriStack;

//视景体
GLFrustum viewFrustum;

//观察者
GLFrame camera;

//变换管道
GLGeometryTransform transformLine;

//自转大球
GLTriangleBatch bigTriangle;
//公转小球
GLTriangleBatch smallTriangle;


//地板
GLBatch floorBatch;


GLfloat green[] = {0.0f,1.0f,0.0f,1.0f};
GLfloat red[] = {1.0f,0.0f,0.0f,1.0f};
GLfloat blue[] = {0.0f,0.0f,1.0f,1.0f};

int  nums = 50;
GLFrame balls[50];

void specailKeys(int key , int x, int y){
    
    if (key == GLUT_KEY_UP)
        camera.MoveForward(0.5f);
    
    
    if (key == GLUT_KEY_DOWN)
        camera.MoveForward(-0.5f);
    
    if (key == GLUT_KEY_LEFT)
        camera.RotateWorld(m3dDegToRad(0.50f),0.0f, 1.0f, 0.f);
    
    if (key == GLUT_KEY_RIGHT)
        camera.RotateWorld(m3dDegToRad(-0.50f),0.0f, 1.0f, 0.f);

}


void changeSize(int w, int h){
    glViewport(0, 0, w, h);
    
    //透视投影
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 500.0f);
    //把视景体矩阵放入投影矩阵堆栈
    projectionMatriStack.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    //模型矩阵堆栈、投影矩阵堆栈放入变换管道
    transformLine.SetMatrixStacks(modelMatriStack, projectionMatriStack);
}

void renderScene(void){


    // 基于时间动画
    static CStopWatch    rotTimer;
    float yRot = rotTimer.GetElapsedSeconds() * 60.0f;
    
    // 清除颜色缓存区和深度缓冲区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
    
    // 绘制地面
    shaderManager.UseStockShader(GLT_SHADER_FLAT,
                                 transformLine.GetModelViewProjectionMatrix(),
                                 green);
    floorBatch.Draw();
    
    
    //将当前的模型视图矩阵压入矩阵堆栈（单位矩阵）
    //因为我们先绘制地面，而地面是不需要有任何变换的。所以在开始渲染时保证矩阵状态，然后在结束时使用相应的PopMatrix恢复它。这样就不必在每一次渲染时重载单位矩阵了。
    modelMatriStack.PushMatrix();
    
    //获取观察者矩阵
    M3DMatrix44f mcamera;
    camera.GetCameraMatrix(mcamera);
    
    modelMatriStack.LoadMatrix(mcamera);
    
    //光源位置
    M3DVector4f vLigh = {1.5f, 0.1f, 1.0f,1.0f};
    M3DVector4f vEyes;
    
    m3dTransformVector3(vEyes, vLigh, mcamera);
    
    
    //绘制随机小球
    for (int i = 0 ; i < nums; i++) {
        
        modelMatriStack.PushMatrix();

//        //modelViewMatrix 顶部矩阵沿着z轴移动2.5单位
//        modelMatriStack.Translate(0.0f, 0.0f, 0.0f);
//
//        //modelViewMatrix 顶部矩阵旋转yRot度
//        modelMatriStack.Rotate(yRot, 0.0f, 1.0f, 0.0f);
        
        modelMatriStack.MultMatrix(balls[i]);
        
//        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformLine.GetModelViewProjectionMatrix(),blue);
       
        //点光源着色器
        shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,transformLine.GetModelViewMatrix(),transformLine.GetProjectionMatrix(),vEyes,blue);
        smallTriangle.Draw();
        modelMatriStack.PopMatrix();
    }
    

    // 绘制旋转甜甜圈
    //modelViewMatrix 顶部矩阵沿着z轴移动2.5单位
    modelMatriStack.Translate(0.0f, 0.0f, -2.5f);
    
    //**保存平移（公转自转）**
    modelMatriStack.PushMatrix();
    
    
    //modelViewMatrix 顶部矩阵旋转yRot度
    modelMatriStack.Rotate(yRot, 0.0f, 1.0f, 0.0f);
 
//    shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT,transformLine.GetModelViewMatrix(),transformLine.GetProjectionMatrix(),red);
    //点光源着色器
    shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,transformLine.GetModelViewMatrix(),transformLine.GetProjectionMatrix(),vEyes,red);
    
    //开始绘制
    bigTriangle.Draw();
    
    // 恢复modelViewMatrix矩阵，移除矩阵堆栈
    //使用PopMatrix推出刚刚变换的矩阵，然后恢复到单位矩阵
    modelMatriStack.PopMatrix();
    
    //**绘制公转球体（公转自转）**
    modelMatriStack.Rotate(yRot * -2.0f, 0.0f, 1.0f, 0.0f);
    modelMatriStack.Translate(0.8f, 0.0f, 0.0f);
//   shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT,transformLine.GetModelViewMatrix(),transformLine.GetProjectionMatrix(),blue);
     shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,transformLine.GetModelViewMatrix(),transformLine.GetProjectionMatrix(),vEyes,blue);

    smallTriangle.Draw();
    
    //**恢复矩阵(公转自转)**
    modelMatriStack.PopMatrix();
    
    
    
    // 执行缓存区交换
    glutSwapBuffers();
    
    // 告诉glut在显示一遍
    glutPostRedisplay();
}

void setup(){
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    camera.MoveForward(-3.f);
    
    shaderManager.InitializeStockShaders();
    
    //深度测试
    glEnable(GL_DEPTH_TEST);
    
    //图形填充模式：正背面填充、线
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    //地板绘制模式、顶点数
    floorBatch.Begin(GL_LINES, 300);
    
    //地板添加顶点数据
    for (GLfloat x = -25.0f; x < 25.0f ; x+=0.5f){
        floorBatch.Vertex3f(x, -0.5f, 20.0f);
        floorBatch.Vertex3f(x, -0.5f, -20.0f);

        floorBatch.Vertex3f(20, -0.5f, x);
        floorBatch.Vertex3f(-20, -0.5f, x);
    }
    
    floorBatch.End();
    
    //绘制甜甜圈
//    gltMakeTorus(bigTriangle, 0.4f, 0.15f, 30, 30);
    gltMakeSphere(bigTriangle, 0.5f, 26, 13);

    
    //** 绘制球(公转自转)**
    gltMakeSphere(smallTriangle, 0.1f, 26, 13);
    
    //添加50个小球
    for (int i  = 0; i < nums ;i++){
        GLfloat x = (float) ((rand() % 400) - 100)*0.1f;
        GLfloat z = (float)  ((rand() % 400) - 100)*0.2f;
        balls[i].SetOrigin(x,0.0f,z);
    }
    
}


int main(int argc, char * argv[]) {
    
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_STENCIL);
    
    glutInitWindowSize(800, 600);
    glutCreateWindow("公转-自转");
    
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
    glutSpecialFunc(specailKeys);
    
    
    GLenum err = glewInit();
    
    if (GLEW_OK != err){
        fprintf(stderr, "初始化出错");
        return 1;
    }
    
    setup();
    
    glutMainLoop();
    
    return 0;
}
