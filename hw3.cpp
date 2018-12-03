#include <GL/glut.h>
#include <cmath>
#include <ctime>
#include <random>
#include <fstream>
#include <vector>
#include "camera.h"
#include "RGBpixmap.h"
#define PI 3.141592653589
using namespace std;

// set global variabes
vector<float> m;
minstd_rand0 rn(time(0));
float cr=180.0,crr=0.0,cx=60.0,cy=0.0,cz=60.0,spin=0.0,mapSize=0.0,center;
int keys[256]; int skeys[256];
int dt, now=0, lastframe=0;
GLfloat lightColor[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat lightPos[] = { -50.0, 25.0, -100.0, 1.0 };
GLfloat lightPos2[] = { -200.0, 1.0, 500.0, 1.0 };
GLfloat lightPos3[] = { 100.0, 1.0, 100.0, 1.0 };
GLfloat wallColor[] = { 1.0, 0.7, 0.3, 1.0 };
GLfloat floorColor[] = { 1.0, 0.7, 0.5, 1.0 };
GLfloat skyColor[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat spinnerr[16];
GLfloat spinnerc[] = { 0.1,1.0,0.3,1.0, 0.1,0.3,1.0,1.0,
                     1.,0.0,0.0,1.0, 0.1,0.0,0.2,1.0 };
RGBpixmap floortex, walltex, skytex;
Camera cam;
Point3 eye(cx,cy,cz);
Point3 look(cx,cy,cz+1.0);
Vector3 up(0.0,1.0,0.0);

//degrees to radians
float d2r(float degree){ return degree * PI / 180.0; }

//collision detector for point+vector and line segment
int collide(int i, float mx, float mz){
  float wvx = m[i+5], wvz = m[i+6], cvx = cx-m[i], cvz = cz-m[i+1]; 
  float cross1 = ((mx+cx-m[i])*wvz - (mz+cz-m[i+1])*wvx)/m[i+4];
  float cross2 = (cvx*wvz - cvz*wvx)/m[i+4];
  //cross products detect line collision, then dots detect line segment bounds
  return (((abs(cross1)<0.5 && abs(cross2)>abs(cross1)) || cross1*cross2<0 ) && 
   ((cvx*wvx + cvz*wvz) * ((cx-m[i+2])*wvx + (cz-m[i+3])*wvz) <0));
}

//render a wall segment
void wallSegment(float sx, float sz, float ex, float ez, float l){
  if (sx < ex) {int t=sx; sx=ex; ex=t;};
  if (sz < ez) {int t=sz; sz=ez; ez=t;};
  glBindTexture(GL_TEXTURE_2D,101);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0,0.1875); glVertex3f(sx,-5.0,sz);
  glTexCoord2f(l/16.0,0.1875); glVertex3f(ex,-5.0,ez);
  glTexCoord2f(l/16.0,0.8125); glVertex3f(ex,5.0,ez);
  glTexCoord2f(0.0,0.8125); glVertex3f(sx,5.0,sz);
  glEnd();
}

//spinning teapots
void spinner(int idx, float x, float z){
  glMaterialfv(GL_FRONT, GL_DIFFUSE, spinnerc+idx);
  glDisable(GL_TEXTURE_2D);
  glPushMatrix();
    glTranslatef(x,0,z);
    glRotatef(spin,spinnerr[idx],spinnerr[idx+1],spinnerr[idx+2]);
    glutSolidTeapot(2);
  glPopMatrix();
  glEnable(GL_TEXTURE_2D);
}

//floor and sky planes
void plain(float scale, float height, int texture, float tiling){
  glPushMatrix();
    glBindTexture(GL_TEXTURE_2D,texture);
    glTranslatef(center,height,center);
    glScalef(scale,1.0,scale);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0,0.0); glVertex3f(-0.5,0.0,-0.5);
    glTexCoord2f(tiling,0.0); glVertex3f(0.5,0.0,-0.5);
    glTexCoord2f(tiling,tiling); glVertex3f(0.5,0.0,0.5);
    glTexCoord2f(0.0,tiling); glVertex3f(-0.5,0.0,0.5);
    glEnd();
  glPopMatrix();
}


void display(void){

  //move camera
  eye.set(cx,cy,cz);
  look.set(cx,cy,cz+1.0);
  cam.setShape(60.0,640.0/480,0.1,3000.0);
  cam.set(eye,look,up);
  cam.yaw(cr);
  cam.pitch(crr);
	
  //lights
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
  glLightfv(GL_LIGHT1, GL_POSITION, lightPos2);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor);
  glLightfv(GL_LIGHT2, GL_POSITION, lightPos3);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, lightColor);

  glClearColor(0.0, 0.0, 0.0, 1.0);  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glEnable(GL_TEXTURE_2D);

  //floor and sky
  glMaterialfv(GL_FRONT, GL_DIFFUSE, floorColor);
  plain(mapSize+20.0,-5.0,100,20.0);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, skyColor);
  plain(mapSize*20.0,150.0,102,1.0);

  //walls
  glMaterialfv(GL_FRONT, GL_DIFFUSE, wallColor);
  for (int i=0; i<m.size(); i+=7)
    wallSegment(m[i],m[i+1],m[i+2],m[i+3],m[i+4]);

  //spinning markers
  spinner(0,90,106);
  spinner(4,74,58);
  spinner(8,8,26);
  spinner(12,140,168);

  glFlush();
  glutSwapBuffers();
}

//idle function
void runner(void){ 
  lastframe = now;
  now = glutGet(GLUT_ELAPSED_TIME);
  dt = now - lastframe;
  spin += 0.1*dt;
  float dot, wx, wz, mvx=0.0, mvz=0.0;

  //control using wasd + jk or using arrow keys
  if (keys['a']) cr+=0.15*dt;
  if (keys['d']) cr-=0.15*dt;
  if (keys['w']){
    mvz += 0.035*dt*cos(d2r(cr));
    mvx += 0.035*dt*sin(d2r(cr)); }
  if (keys['s']){
    mvz -= 0.035*dt*cos(d2r(cr));
    mvx -= 0.035*dt*sin(d2r(cr)); }
  if (keys['j']){
    mvz += 0.025*dt*cos(d2r(cr+90));
    mvx += 0.025*dt*sin(d2r(cr+90)); }
  if (keys['k']){
    mvz -= 0.025*dt*cos(d2r(cr+90));
    mvx -= 0.025*dt*sin(d2r(cr+90)); }

  // arrow keys
  if (skeys[GLUT_KEY_UP]){
    mvz += 0.035*dt*cos(d2r(cr));
    mvx += 0.035*dt*sin(d2r(cr)); }
  if (skeys[GLUT_KEY_DOWN]){
    mvz -= 0.035*dt*cos(d2r(cr));
    mvx -= 0.035*dt*sin(d2r(cr)); }
  if (skeys[GLUT_KEY_LEFT]) cr+=0.15*dt;
  if (skeys[GLUT_KEY_RIGHT]) cr-=0.15*dt;

  // fly and look up or down
  if (keys['f']) crr+=0.15*dt;
  if (keys['r']) crr-=0.15*dt;
  if (keys['z']) cy+=0.025*dt;
  if (keys['x']) cy-=0.025*dt;


  // change movement vector when collision detected
  for (int i=0; i<m.size(); i+=7){
    if (collide(i,mvx,mvz)){
      wx = m[i+5]/m[i+4];
      wz = m[i+6]/m[i+4];
      dot = mvx*wx + mvz*wz;
      mvx = dot * wx;
      mvz = dot * wz;
    }
  }
  cx += mvx; cz += mvz;

  glutPostRedisplay();
} 

//use key arrays for multiple keypresses
void keydown(unsigned char key,int x,int y){ keys[key] = 1; if (key=='q') exit(0); }
void keyup(unsigned char key, int x, int y){ keys[key] = 0; }
void skeydown(int key, int x, int y)      { skeys[key] = 1; }
void skeyup(int key, int x, int y)        { skeys[key] = 0; }

int main(int argc, char **argv){
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(800,600);
  glutInitWindowPosition(20, 10);
  glutCreateWindow("Maze");
  glutDisplayFunc(display);
  glutIdleFunc(runner);
  glutKeyboardFunc(keydown);
  glutSpecialFunc(skeydown);
  glutSpecialUpFunc(skeyup);
  glutKeyboardUpFunc(keyup);
  glEnable(GL_LIGHTING); 
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHT2);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);

  // load textures
  floortex.readBMPFile(string("floor.bmp"));
  floortex.setTexture(100);
  walltex.readBMPFile(string("wall.bmp"));
  walltex.setTexture(101);
  skytex.readBMPFile(string("sky.bmp"));
  skytex.setTexture(102);

  // load maze data
  ifstream mazedat("maze.txt");
  float a,b,c,d;
  while (mazedat >> a >> b >> c >> d){ 
    m.push_back(a); 
    m.push_back(b); 
    m.push_back(c); 
    m.push_back(d); 
    m.push_back(sqrt((a-c)*(a-c)+(b-d)*(b-d))); 
    m.push_back(a-c); 
    m.push_back(b-d); 
    if (a>mapSize) mapSize=a; 
    if (b>mapSize) mapSize=b; 
    if (c>mapSize) mapSize=c; 
    if (d>mapSize) mapSize=d; 
  }
  mapSize+=2;
  cx=cz=center=mapSize/2;

  // set keyboard and spinner arrays and start loop
  for(int i=0;i<256;i++)skeys[i]=keys[i]=0;
  for(int i=0;i<16;i++){spinnerr[i]=(float)(rn()%1000)/1000; rn.seed(rn());}
  glutMainLoop();
  return 0;
} 



