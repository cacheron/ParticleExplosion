/* Chaz Acheronti */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <random>
#include <GL/glut.h>
#include <iostream>

typedef struct vec3 {
	GLfloat x, y, z;
} vec3;

typedef struct color4f {
	GLfloat r, g, b, a;
} color4f;

typedef struct particle {
	vec3 position;
	vec3 velocity;
	color4f color;
	float lifetime;
	float uptime;
	bool active;
} particle;

  /////////////////////////////////
 //		SIMULATION SETTINGS		//
/////////////////////////////////

 // Physics Variables
double GRAVITY = 9.78;
double TIMESCALE = 1.0; // scale time to be normal (1.0), faster (> 1.0), or slower (> 1.0)

// Camera Settings
vec3 cam_velocity{ 0.0f, 0.0f, -100.0f };
vec3 cam_bounds{ 0.0f, 0.0f, -50.0f };
vec3 cam_pos{ 0.0f, 0.0f, -12.0f };

// particle settings
bool cube_exploded = false;
long particle_count = 70000;
float lifetime = 35.0f;
vec3 global_velocity = { 100.0f, 100.0f, 100.0f};

// set the range of random particle colors from min to max
color4f color_min{ 0.3f, 0.2f, 0.6f, 1.0f };
color4f color_max{ 0.6f, 0.5f, 1.0f, 1.0f };

// example: red is 0, but min value of 0.5 guarentees some red in the final random color

double ground = -10.0;

// particle list
std::vector<particle> particles(particle_count, { { 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f, 1.0f }, lifetime, 0.0f, false });

// delta time measuring
long old_time;
double delta_time;

// camera rotation angle
float angle = 0.0; 

// Light sources
GLfloat  light0Amb[4] = { 1.0, 0.6, 0.2, 1.0 };
GLfloat  light0Dif[4] = { 1.0, 0.6, 0.2, 1.0 };
GLfloat  light0Spec[4] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat  light0Pos[4] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat  light1Amb[4] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat  light1Dif[4] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat  light1Spec[4] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat  light1Pos[4] = { 0.0, 5.0, 5.0, 0.0 };


void init_particle(particle& particle) {
	// init random position in unit cube (-0.5 --> 0.5)
	particle.position.x = (((float)rand()) / float(RAND_MAX)) - 0.5f;
	particle.position.y = (((float)rand()) / float(RAND_MAX)) - 0.5f;
	particle.position.z = (((float)rand()) / float(RAND_MAX)) - 0.5f;

	// init random velocity direction (-1 to 1 unit vector)
	particle.velocity.x = ((((float)rand()) / float(RAND_MAX)) * 2.0) - 1.0f; 
	particle.velocity.y = ((((float)rand()) / float(RAND_MAX)) * 2.0) - 1.0f;
	particle.velocity.z = ((((float)rand()) / float(RAND_MAX)) * 2.0) - 1.0f;

	// init random color (with a minimum brightness so no blacks)
	particle.color.r = ((((float)rand()) / float(RAND_MAX)) * (color_max.r - color_min.r)) + color_min.r; // minimum brightness of 0.5f
	particle.color.g = ((((float)rand()) / float(RAND_MAX)) * (color_max.g - color_min.g)) + color_min.g;
	particle.color.b = ((((float)rand()) / float(RAND_MAX)) * (color_max.b - color_min.b)) + color_min.b;
	particle.color.a = 1.0f;

	particle.uptime = 0.0f;

	// leave inactive until spacebar is pressed
	particle.active = false;
}


void init_particles() {	
	// initialize particles
	for (int i = 0; i < particles.size(); i++) {
		init_particle(particles[i]);
		
	}
}


// render one particle
void render_particle(particle& particle) {
	// render the particle as a pyramid
	if (particle.active) {
		vec3 p1{ particle.position.x, particle.position.y, particle.position.z };
		vec3 p2{ particle.position.x, particle.position.y, particle.position.z + 0.1f };
		vec3 p3{ particle.position.x + 0.1f, particle.position.y, particle.position.z + 0.1f };
		vec3 p4{ particle.position.x + 0.1f, particle.position.y, particle.position.z };
		vec3 p5{ particle.position.x + 0.05f, particle.position.y + 0.1f, particle.position.z + 0.05f};
		// face 1
		glBegin(GL_POLYGON);
		glColor4f(particle.color.r, particle.color.g, particle.color.b, particle.color.a);
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p2.x, p2.y, p2.z);
		glVertex3f(p3.x, p3.y, p3.z);
		glVertex3f(p4.x, p4.y, p4.z); 
		glEnd();

		// face 2
		glBegin(GL_POLYGON);
		glColor4f(particle.color.r, particle.color.g, particle.color.b, particle.color.a);
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p2.x, p2.y, p2.z);
		glVertex3f(p5.x, p5.y, p5.z);
		glEnd();

		// face 3
		glBegin(GL_POLYGON);
		glColor4f(particle.color.r, particle.color.g, particle.color.b, particle.color.a);
		glVertex3f(p2.x, p2.y, p2.z);
		glVertex3f(p3.x, p3.y, p3.z);
		glVertex3f(p5.x, p5.y, p5.z);
		glEnd();

		// face 4
		glBegin(GL_POLYGON);
		glColor4f(particle.color.r, particle.color.g, particle.color.b, particle.color.a);
		glVertex3f(p3.x, p3.y, p3.z);
		glVertex3f(p4.x, p4.y, p4.z);
		glVertex3f(p5.x, p5.y, p5.z);
		glEnd();

		// face 5
		glBegin(GL_POLYGON);
		glColor4f(particle.color.r, particle.color.g, particle.color.b, particle.color.a);
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p4.x, p4.y, p4.z);
		glVertex3f(p5.x, p5.y, p5.z);
		glEnd();
	}
}

// render all particles
void render_particles() {
	for (int i = 0; i < particles.size(); i++) {
		render_particle(particles[i]);
	}
}



// physics update of one particle
void update_particle(particle& particle) {	
	if (particle.active) {
		// check if the praticle should be inactive
		if (particle.uptime >= particle.lifetime) {
			particle.active = false;
			return;
		}
		particle.uptime += delta_time;

		// update position
		particle.position.x += particle.velocity.x * global_velocity.x * delta_time;
		particle.velocity.y -= GRAVITY * delta_time;
		particle.position.y += particle.velocity.y * global_velocity.y * delta_time;
		if (particle.position.y <= ground) {
			particle.position.y = ground;
			particle.velocity.x = 0.0f;
			particle.velocity.y = 0.0f;
			particle.velocity.z = 0.0f;
		}
		particle.position.z += particle.velocity.z * global_velocity.z * delta_time;

		// calculate lifetime ratio to change color
		float lifetime_ratio = (particle.lifetime - particle.uptime) / particle.lifetime;
		if (lifetime_ratio < 0.0f) lifetime_ratio = 0.0f;
		
		// update color BROKEN
		particle.color.r *= lifetime_ratio;
		particle.color.g *= lifetime_ratio;
		particle.color.b *= lifetime_ratio;

	}
	
}


// update all particles
void update_particles() {
	for (int i = 0; i < particles.size(); i++) {
		update_particle(particles[i]);
	}
}

void active_particles() {
	for (int i = 0; i < particles.size(); i++) {
		particles[i].active = true;
	}
}

void init() {
	// init dt
	old_time = glutGet(GLUT_ELAPSED_TIME);

	// init particles
	init_particles();
}


void reset() {
	cube_exploded = false;
	init_particles();
	cam_pos.z = -12.0f;
}


// render function
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	// Place the camera
	if (cube_exploded) {
		cam_pos.x += cam_velocity.x * delta_time;
		cam_pos.y += cam_velocity.y * delta_time;
		cam_pos.z += cam_velocity.z * delta_time;

		// prevent out of bounds movement
		if (cam_pos.z < cam_bounds.z) cam_pos.z = cam_bounds.z;
	}

	glTranslatef(cam_pos.x, cam_pos.y, cam_pos.z);
	//glRotatef(angle, 0.0, 10.0, 0.0);
	
	// If no explosion, draw cube
	if (!cube_exploded)
	{
		glEnable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_LIGHT0);
		glDisable(GL_LIGHTING);
		glColor3f(color_min.r, color_min.g, color_min.b);
		glutSolidCube(1.0);
	}
	else {
		// render the particles
		render_particles();
	}

	glutSwapBuffers();
}



void explode_cube(void) {
	if (!cube_exploded) {
		cube_exploded = true;
		active_particles();
		printf("BOOM!\n");
	}
}


void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case ' ':
		explode_cube();
		break;
	case 'r':
		reset();
		break;
	case 27:
		exit(0);
		break;
	}
}


void reshape(int w, int h) {
	glViewport(0.0, 0.0, (GLfloat)w, (GLfloat)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (GLfloat)w / (GLfloat)h, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
}


void idle(void) {
	angle += 0.3;  /* Always continue to rotate the camera */
	
	// update delta time
	long new_time;

	new_time = glutGet(GLUT_ELAPSED_TIME);
	delta_time = (new_time - old_time) / 1000.0;
	delta_time *= TIMESCALE * 0.07;

	old_time = new_time;

	// update particles
	update_particles();

	glutPostRedisplay();
}



int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(1280, 1024);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("T H E   B I G   B A N G   -   Chaz Acheronti");
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0Amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Dif);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0Spec);
	glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light1Amb);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Dif);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light1Spec);
	glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glEnable(GL_NORMALIZE);
	init();
	glutMainLoop();
	return 0;
}
