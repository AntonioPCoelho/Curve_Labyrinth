//
//  InstanciaBZ.cpp
//  Created by Márcio Sarroglia Pinho on 22/09/20.
//

#include "InstanciaBZ.h"

// ***********************************************************
//  void InstanciaPonto(Ponto3D *p, Ponto3D *out)
//  Esta funcao calcula as coordenadas de um ponto no
//  sistema de referencia do universo (SRU), ou seja,
//  aplica as rotacoes, escalas e translacoes a um
//  ponto no sistema de referencia do objeto (SRO).
// ***********************************************************
void InstanciaPonto(Ponto &p, Ponto &out)
{
    GLfloat ponto_novo[4];
    GLfloat matriz_gl[4][4];
    int  i;
    
    glGetFloatv(GL_MODELVIEW_MATRIX,&matriz_gl[0][0]);
    
    for(i=0;i<4;i++)
    {
        ponto_novo[i]= matriz_gl[0][i] * p.x+
        matriz_gl[1][i] * p.y+
        matriz_gl[2][i] * p.z+
        matriz_gl[3][i];
    }
    out.x=ponto_novo[0];
    out.y=ponto_novo[1];
    out.z=ponto_novo[2];
}

Ponto InstanciaPonto(Ponto P)
{
    Ponto temp;
    InstanciaPonto(P, temp);
    return temp;
}

InstanciaBZ::InstanciaBZ()
{
    
    Rotacao = 0;
    Posicao = Ponto(0,0,0);
    Escala = Ponto(1,1,1);
    
    nroDaCurva = 0;
    proxCurva = -1;
    tAtual = 0.0;
    direcao = 1;
}

InstanciaBZ::InstanciaBZ(Bezier C)
{
    
    Rotacao = 0;
    Posicao = Ponto(0,0,0);
    Escala = Ponto(1,1,1);
    
    Curva = C;
    tAtual = 0;
    direcao = 1;
}

void InstanciaBZ::desenha()
{
    // Escala.imprime("Escala: ");
    // cout << endl;
    // Aplica as transformacoes geometricas no modelo
    glPushMatrix();
        glTranslatef(Posicao.x, Posicao.y, 0);
        glRotatef(Rotacao, 0, 0, 1);
        glScalef(Escala.x, Escala.y, Escala.z);
        
        (*modelo)(); // desenha a instancia
        
    glPopMatrix();
}
Ponto InstanciaBZ::ObtemPosicao()
{
    // aplica as transformacoes geometricas no modelo
    // desenha a geometria do objeto
    
    glPushMatrix();
        glTranslatef(Posicao.x, Posicao.y, 0);
        glRotatef(Rotacao, 0, 0, 1);
        Ponto PosicaoDoPersonagem;
        Ponto Origem (0,0,0);
        InstanciaPonto(Origem, PosicaoDoPersonagem);
        //PosicaoDoPersonagem.imprime(); cout << endl;
    glPopMatrix();
    return PosicaoDoPersonagem;
}

void InstanciaBZ::AtualizaPosicao(float tempoDecorrido) {
    tAtual += direcao * tempoDecorrido;

    if (tAtual >= 1.0 || tAtual <= 0.0) {
        direcao *= -1;
        tAtual = std::max(0.0f, std::min(1.0f, tAtual));

        // Encontrar curvas possíveis que intersectam com a curva atual na posição atual
        std::vector<int> possibleCurves;
        Ponto currentPos = Curva.Calcula(tAtual);
        for (int i = 0; i < Curvas.size(); ++i) {
            if (Curvas[i].getPC(0) == currentPos || Curvas[i].getPC(2) == currentPos) {
                possibleCurves.push_back(i);
            }
        }

        // Selecionar aleatoriamente uma nova curva a partir das curvas possíveis
        if (!possibleCurves.empty()) {
            int nextCurveIndex = possibleCurves[rand() % possibleCurves.size()];
            Curva = Curvas[nextCurveIndex];
            tAtual = 0.0; // Reiniciar tAtual para começar do início da nova curva
        }
    }

    Ponto P = Curva.Calcula(tAtual);
    Posicao = P;
}