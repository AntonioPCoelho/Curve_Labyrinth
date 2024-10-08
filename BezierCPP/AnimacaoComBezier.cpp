// **********************************************************************
// PUCRS/Escola Politecnica
// COMPUTACAO GRAFICA
//
// Programa basico para criar aplicacoes 2D em OpenGL
//
// Antonio Pasquali Coelho
// antonio.pasquali@edu.pucrs.br
// **********************************************************************

// Para uso no Xcode:
// Abra o menu Product -> Scheme -> Edit Scheme -> Use custom working directory
// Selecione a pasta onde voce descompactou o ZIP que continha este arquivo.
//

#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>
#include <vector>
#include <random>

using namespace std;

#ifdef WIN32
#include <windows.h>
#include <GL/glut.h>
#else
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif

#ifdef __linux__
#include <GL/glut.h>
#endif

#include "Ponto.h"
#include "Poligono.h"
#include "InstanciaBZ.h"

#include "Temporizador.h"
#include "ListaDeCoresRGB.h"

Temporizador T;
double AccumDeltaT = 0;
Temporizador T2;

InstanciaBZ Personagens[11];

std::vector<Bezier> Curvas;
unsigned int nCurvas;
int currentCurve = 0; // Curva atual
int selectedCurve = 0; // Curva selecionada

// Limites logicos da area de desenho
Ponto Min, Max;

bool desenha = false;

//Poligono Mapa, MeiaSeta, Mastro, Controle;
int nInstancias = 0;

float angulo = 160.0;

double nFrames = 0;
double TempoTotal = 0;

float pos = 0.0f; // Posicao do jogador
bool movingState = false; // Estado de movimento do jogador

// Checar colisao entre dois pontos
bool checaColisao(Ponto playerPos, Ponto enemyPos, float limite = 1.0f) {
    float dist = sqrt(pow(playerPos.x - enemyPos.x, 2) + pow(playerPos.y - enemyPos.y, 2));
    return dist < limite;
}


// Metodo alterado que le as curvas de um arquivo texto
void CriaCurvas()
{
    std::ifstream coordFile("Coordenadas.txt");
    std::ifstream curvasFile("Curvas.txt");

    if (!coordFile.is_open() || !curvasFile.is_open()) {
        std::cerr << "Erro ao abrir o arquivo" << std::endl;
        return;
    }

    int numCoords;
    coordFile >> numCoords;
    std::vector<Ponto> pontos;
    for (int i = 0; i < numCoords; ++i) {
        float x, y;
        coordFile >> x >> y;
        pontos.emplace_back(x, y);
    }

    int numCurvas;
    curvasFile >> numCurvas;
    Curvas.resize(numCurvas);
    for (int i = 0; i < numCurvas; ++i) {
        int idx1, idx2, idx3;
        curvasFile >> idx1 >> idx2 >> idx3;
        Curvas[i] = Bezier(pontos[idx1], pontos[idx2], pontos[idx3]);
    }

    nCurvas = numCurvas;
}

void AssociaPersonagemComCurva(int p, int c) {
    Personagens[p].Curva = Curvas[c];
    Personagens[p].tAtual = 0.5;
    Personagens[p].direcao = 1;
    Personagens[p].currentCurve = c;
}

void animate()
{
    double dt;
    dt = T.getDeltaT();
    AccumDeltaT += dt;
    TempoTotal += dt;
    nFrames++;

    if (AccumDeltaT > 1.0 / 30) // fixa a atualizacao da tela em 30
    {
        AccumDeltaT = 0;
        angulo += 2;
        //Personagens[0].Rotacao++;
        glutPostRedisplay();
    }
    if (TempoTotal > 5.0)
    {
        cout << "Tempo Acumulado: " << TempoTotal << " segundos. ";
        cout << "Nros de Frames sem desenho: " << nFrames << endl;
        cout << "FPS(sem desenho): " << nFrames / TempoTotal << endl;
        TempoTotal = 0;
        nFrames = 0;
    }
}

//  trata o redimensionamento da janela OpenGL
void reshape(int w, int h)
{
    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Define a area a ser ocupada pela area OpenGL dentro da Janela
    glViewport(0, 0, w, h);
    // Define os limites logicos da area OpenGL dentro da Janela
    glOrtho(Min.x, Max.x, Min.y, Max.y, -10, +10);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void DesenhaEixos()
{
    Ponto Meio;
    Meio.x = (Max.x + Min.x) / 2;
    Meio.y = (Max.y + Min.y) / 2;
    Meio.z = (Max.z + Min.z) / 2;

    glBegin(GL_LINES);
    //  eixo horizontal
    glVertex2f(Min.x, Meio.y);
    glVertex2f(Max.x, Meio.y);
    //  eixo vertical
    glVertex2f(Meio.x, Min.y);
    glVertex2f(Meio.x, Max.y);
    glEnd();
}


void DesenhaTriangulo()
{
    glBegin(GL_TRIANGLES);
        glVertex2f(-1,-1);
        glVertex2f(0, 1);
        glVertex2f(1,-1);
    glEnd();
}

void DesenhaPacMan()
{
    defineCor(Gold);
    float radius = 0.8f;
    int numSegments = 100;
    float angleStep = 2.0f * M_PI / numSegments;
    float mouthAngle = M_PI / 4.0f; // Ângulo da boca

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.0f); // Centro do círculo

    for (int i = 0; i <= numSegments; ++i)
    {
        float angle = i * angleStep;
        if (angle > mouthAngle && angle < 2.0f * M_PI - mouthAngle)
        {
            glVertex2f(radius * cos(angle), radius * sin(angle));
        }
    }
    glEnd();
}

// Esta funcao deve instanciar todos os personagens do cenario
void CriaInstancias()
{
    // Criação do veículo do jogador (Pac-Man)
    Personagens[0].Posicao = Ponto(0, 0);
    Personagens[0].Rotacao = 0;
    Personagens[0].modelo = DesenhaPacMan;
    Personagens[0].Escala = Ponto(1, 1, 1);

    // Criação dos veículos inimigos
    for (int i = 1; i <= 10; ++i) {
        // Seleciona uma curva aleatória
        int curvaAleatoria = rand() % nCurvas;
        
        // Coloca o veículo no meio da curva
        Personagens[i].Posicao = Curvas[curvaAleatoria].Calcula(0.5);
        Personagens[i].Rotacao = 0;
        Personagens[i].modelo = DesenhaTriangulo;  // Forma do inimigo
        Personagens[i].Escala = Ponto(1, 1, 1);

        // Para metade dos inimigos, o movimento será em direção ao ponto inicial da curva
        Personagens[i].direcao = (i % 2 == 0) ? -1 : 1;

        // Associa o inimigo com a curva escolhida (agora que as curvas já foram criadas)
        AssociaPersonagemComCurva(i, curvaAleatoria);
    }

    nInstancias = 11; // Atualizado para incluir o jogador e 10 inimigos
}

void init()
{
    srand(time(NULL));
    // Define a cor do fundo da tela (AZUL)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // carrega as curvas que farao parte do cenario
    CriaCurvas();
    
    // cria instancias do modelos
    CriaInstancias();
    
    AssociaPersonagemComCurva(0, 0);

    // define is limites da área de desenho
    float d = 15;
    Min = Ponto(-d, -d);
    Max = Ponto(d, d);
}

// Função para atualizar a posição dos personagens
void DesenhaPersonagens(float tempoDecorrido) {
    for (int i = 0; i < nInstancias; i++) {
        if (movingState) {
            Personagens[i].AtualizaPosicao(tempoDecorrido);
            // Personagens[0] eh o jogador
            if (i != 0 && checaColisao(Personagens[0].Posicao, Personagens[i].Posicao)) {
                std::cout << "Colisao detectada! Fim de Jogo(Game Over)." << std::endl;
                exit(0); // Encerra o jogo
            }
        }
        defineCor(Personagens[i].cor);
        Personagens[i].desenha();
    }
}

void DesenhaPoligonoDeControle(int curva)
{
    Ponto P;
    glBegin(GL_LINE_STRIP);
    for (int v=0;v<3;v++)
    {
        P = Curvas[curva].getPC(v);
        glVertex2d(P.x, P.y);
    }
    glEnd();
}

void DesenhaCurvas() {
    for (int i = 0; i < nCurvas; ++i) {
        glLineWidth(2.0);
        if (i == Personagens[0].currentCurve) {
            defineCor(NeonPink);
            glLineWidth(4.0);
        } else {
            defineCor(OrangeRed);
        }

        glBegin(GL_LINE_STRIP);
        for (float t = 0; t <= 1.0; t += 0.01) {
            Ponto p = Curvas[i].Calcula(t);
            glVertex2f(p.x, p.y);
        }
        glEnd();
    }
}


void display(void)
{

    // Limpa a tela coma cor de fundo
    glClear(GL_COLOR_BUFFER_BIT);

    // Define os limites l�gicos da �rea OpenGL dentro da Janela
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    // Coloque aqui as chamadas das rotinas que desenham os objetos
    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    glLineWidth(1);
    glColor3f(1, 1, 1); // R, G, B  [0..1]

    DesenhaEixos();

    //AtualizaCenario();
    DesenhaCurvas();
    
    // Desenha os personagens no tempo T2.getDeltaT()
    DesenhaPersonagens(T2.getDeltaT());

    glutSwapBuffers();
}
// **********************************************************************
// ContaTempo(double tempo)
//      conta um certo numero de segundos e informa quanto frames
// se passaram neste periodo.
// **********************************************************************
void ContaTempo(double tempo)
{
    Temporizador T;

    unsigned long cont = 0;
    cout << "Inicio contagem de " << tempo << "segundos ..." << flush;
    while (true)
    {
        tempo -= T.getDeltaT();
        cont++;
        if (tempo <= 0.0)
        {
            cout << "fim! - Passaram-se " << cont << " frames." << endl;
            break;
        }
    }
}

// **********************************************************************
//  void keyboard ( unsigned char key, int x, int y )
// **********************************************************************
void keyboard(unsigned char key, int x, int y)
{
    Ponto P;
    static std::vector<int> possibleCurves;
    static int currentCurveIndex = 0;

    switch (key)
    {
        case ' ': // ESPACO
            movingState = !movingState;
            break;
        case 'n':  // Pressione 'n' para mudar de curva manualmente
            selectedCurve = (selectedCurve + 1) % nCurvas;
            AssociaPersonagemComCurva(0, selectedCurve);
            break;
        case 'd': // d altera a direcao
            Personagens[0].direcao *= -1; // Inverte a direção
            break;
        case 'a':
            P = Curvas[1].Calcula(0);
            P.imprime("Pra t==0 :");
            break;
        case 27:     // Termina o programa qdo
            exit(0); // a tecla ESC for pressionada
            break;
        case 't':
            ContaTempo(3);
            break;
        default:
            break;
    }
}
// **********************************************************************
//  void arrow_keys ( int a_keys, int x, int y )
// **********************************************************************
void arrow_keys(int a_keys, int x, int y)
{
    switch (a_keys)
    {
    case GLUT_KEY_LEFT:
            Personagens[1].Rotacao+=2;
        break;
    case GLUT_KEY_RIGHT:
        Personagens[1].Rotacao-=2;
        break;
    case GLUT_KEY_UP:     // Se pressionar UP
        glutFullScreen(); // Vai para Full Screen
        break;
    case GLUT_KEY_DOWN: // Se pressionar UP
                        // Reposiciona a janela
        glutPositionWindow(50, 50);
        glutReshapeWindow(700, 500);
        break;
    default:
        break;
    }
}

// **********************************************************************
//  void main ( int argc, char** argv )
//
// **********************************************************************
int main(int argc, char **argv)
{
    cout << "Programa OpenGL: Aperte ESPACO para iniciar" << endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowPosition(0, 0);

    // Define o tamanho inicial da janela grafica do programa
    glutInitWindowSize(650, 500);

    // Cria a janela na tela, definindo o nome da
    // que aparecera na barra de titulo da janela.
    glutCreateWindow("Animacao com Bezier");

    // executa algumas inicializacoes
    init();

    // Define que o tratador de evento para
    // o redesenho da tela. A funcao "display"
    // sera chamada automaticamente quando
    // for necessario redesenhar a janela
    glutDisplayFunc(display);

    // Define que o tratador de evento para
    // o invalidacao da tela. A funcao "display"
    // sera chamada automaticamente sempre que a
    // maquina estiver ociosa (idle)
    glutIdleFunc(animate);

    // Define que o tratador de evento para
    // o redimensionamento da janela. A funcao "reshape"
    // sera chamada automaticamente quando
    // o usuario alterar o tamanho da janela
    glutReshapeFunc(reshape);

    // Define que o tratador de evento para
    // as teclas. A funcao "keyboard"
    // sera chamada automaticamente sempre
    // o usuario pressionar uma tecla comum
    glutKeyboardFunc(keyboard);

    // Define que o tratador de evento para
    // as teclas especiais(F1, F2,... ALT-A,
    // ALT-B, Teclas de Seta, ...).
    // A funcao "arrow_keys" sera chamada
    // automaticamente sempre o usu�rio
    // pressionar uma tecla especial
    glutSpecialFunc(arrow_keys);

    // inicia o tratamento dos eventos
    glutMainLoop();

    return 0;
}