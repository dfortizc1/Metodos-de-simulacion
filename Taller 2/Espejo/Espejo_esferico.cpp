﻿#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;

const int Lx=500;
const int Ly=200;

const int Q=5;
const double W0=1.0/3;

//const double C=0.5; // C<0.707 celdas/click
//const double C2=C*C;
//const double TresC2=3*C2;
//const double AUX0=1-TresC2*(1-W0);
//const double AUX =1.0-(5.0/3.0)*C2;

const double tau=0.5;
const double Utau=1.0/tau;
const double UmUtau=1-Utau;

class LatticeBoltzmann{
private:
  double w[Q];
  int V[2][Q];  
  double f[Lx][Ly][Q],fnew[Lx][Ly][Q]; // f[ix][iy][i]
public:
  LatticeBoltzmann(void);
  double rho(int ix,int iy,bool UseNew);
  double Jx(int ix,int iy,bool UseNew);
  double Jy(int ix,int iy,bool UseNew);
  double feq(int ix,int iy, int i,double rho0,double Jx0,double Jy0);
  void Inicie(double rho0,double Jx0,double Jy0);
  void ImponerCampos(int t);
  void Colisione(void);
  void Adveccione(void);
  void Imprimase(char const * NombreArchivo);
  void ImprimaUnaLinea(char const * NombreArchivo,int t);
  
  //Velocidad de la onda.
  double Ccelda(int ix, int iy);
  bool nodo(int ix, int iy);
  
};
LatticeBoltzmann::LatticeBoltzmann(void){
  w[0]=W0;
  w[1]=w[2]=w[3]=w[4]=1.0/6;

  V[0][0]=0;  
  V[1][0]=0;

  V[0][1]=1;    V[0][2]=0;    V[0][3]=-1;   V[0][4]=0;  
  V[1][1]=0;    V[1][2]=1;    V[1][3]=0;    V[1][4]=-1;  
}
double LatticeBoltzmann::rho(int ix,int iy,bool UseNew){
  int i; double suma;
  for(suma=0,i=0;i<Q;i++)
    if(UseNew)
      suma+=fnew[ix][iy][i];
    else
      suma+=f[ix][iy][i];
  return suma;
}
double LatticeBoltzmann::Jx(int ix,int iy,bool UseNew){
  int i; double suma;
  for(suma=0,i=0;i<Q;i++)
     if(UseNew)    suma+=V[0][i]*fnew[ix][iy][i];
     else suma+=V[0][i]*f[ix][iy][i];
  return suma;
}
double LatticeBoltzmann::Jy(int ix,int iy,bool UseNew){
  int i; double suma;
  for(suma=0,i=0;i<Q;i++)
    if(UseNew) suma+=V[1][i]*fnew[ix][iy][i];
    else  suma+=V[1][i]*f[ix][iy][i];
  return suma;
}
double LatticeBoltzmann::feq(int ix, int iy, int i,double rho0,double Jx0,double Jy0){
  double C=Ccelda(ix, iy), C2=C*C, TresC2=3*C2, AUX0=1-TresC2*(1-W0);
  if(i==0)
    return AUX0*rho0;
    else
    return w[i]*(TresC2*rho0+3*(V[0][i]*Jx0+V[1][i]*Jy0));
}
void LatticeBoltzmann::ImponerCampos(int t){
  double A=10,lambda=10,omega,rho0, Jx0, Jy0; //fuente
  int ix=0;
  for(int iy=0; iy<Ly; iy++){
    omega = 2*M_PI*Ccelda(ix,iy)/lambda;  rho0 = A*sin(omega*t);
    Jx0 = Jx(ix,iy,false);  Jy0 = Jy(ix,iy,false);
    for(int i=0; i<Q; i++){
      fnew[ix][iy][i]=feq(ix,iy,i,rho0,Jx0,Jy0);
    }
  }
}

void LatticeBoltzmann::Inicie(double rho0,double Jx0,double Jy0){
  int ix,iy,i;
  for(ix=0;ix<Lx;ix++)
    for(iy=0;iy<Ly;iy++)
      for(i=0;i<Q;i++)
		f[ix][iy][i]=feq(ix, iy, i,rho0,Jx0,Jy0);
}
void LatticeBoltzmann::Colisione(void){
 int ix,iy,i; double rho0,Jx0,Jy0;
 for(iy=0;iy<Ly;iy++){
  for(ix=0;ix<Lx;ix++){
    if(nodo(ix+1, iy)) break;
    rho0=rho(ix,iy,false);  Jx0=Jx(ix,iy,false);  Jy0=Jy(ix,iy,false); //Calculo campos
	for(i=0;i<Q;i++) //para cada dirección
      fnew[ix][iy][i]=UmUtau*f[ix][iy][i]+Utau*feq(ix, iy, i,rho0,Jx0,Jy0); //evoluciono
    }
  }
}
void LatticeBoltzmann::Adveccione(void){
  int ix,iy,i;
  for(iy=0;iy<Ly;iy++){
    for(ix=0;ix<Lx;ix++){
      for(i=0;i<Q;i++){
        if(nodo(ix+1, iy)){
          if(i==0) {f[(ix+V[0][i]+Lx)%Lx][(iy-V[1][i]+Ly)%Ly][i]=fnew[ix][iy][i];}
          if(i==1) {f[ix][iy][i+2]=fnew[ix][iy][i];}
          if(i==2 and nodo(ix, (iy-1+Ly)%Ly)) {f[ix][iy][i+2]=fnew[ix][iy][i];}
          else{f[(ix+V[0][i]+Lx)%Lx][(iy+V[1][i]+Ly)%Ly][i]=fnew[ix][iy][i];}
          if(i==3) {f[(ix+V[0][i]+Lx)%Lx][(iy+V[1][i]+Ly)%Ly][i]=fnew[ix][iy][i];}
          if(i==4 and nodo(ix, (iy+1+Ly)%Ly)){f[ix][iy][i-2]=fnew[ix][iy][i];}
          else{f[(ix+V[0][i]+Lx)%Lx][(iy+V[1][i]+Ly)%Ly][i]=fnew[ix][iy][i];}
        }
        else{f[(ix+V[0][i]+Lx)%Lx][(iy+V[1][i]+Ly)%Ly][i]=fnew[ix][iy][i];}
      }
      if(nodo(ix+1, iy)) break;
    }
  }
}
void LatticeBoltzmann::Imprimase(char const * NombreArchivo){
  ofstream MiArchivo(NombreArchivo); double rho0,Jx0,Jy0;
  for(int ix=0;ix<Lx/2.0;ix++){
    for(int iy=0;iy<Ly;iy++){
      rho0=rho(ix,iy,true);
      MiArchivo<<ix<<" "<<iy<<" "<<rho0<<endl;
    }
    MiArchivo<<endl;
  }
  MiArchivo.close();
}
void LatticeBoltzmann::ImprimaUnaLinea(char const * NombreArchivo,int t){
  ofstream MiArchivo(NombreArchivo); double rho0,Jx0,Jy0;
  //Para y
  /*
  int ix=Lx/4.0;
  for(int iy=0;iy<Ly;iy++){
    rho0=rho(ix,iy,true);   Jx0=Jx(ix,iy);  Jy0=Jy(ix,iy);
    ImponerCampos(ix,iy,rho0,Jx0,Jy0,t);
    MiArchivo << iy << " " << rho0 << endl;
  }
  */
  
  //Para X
  int iy=Ly/2.0;
  for(int ix=0;ix<Lx/2.0;ix++){
    rho0=rho(ix,iy,true);   Jx0=Jx(ix,iy,false);  Jy0=Jy(ix,iy,false);
    ImponerCampos(t);
    MiArchivo << ix << " " << rho0 << endl;
  }
  MiArchivo.close();
}

double LatticeBoltzmann::Ccelda(int ix, int iy){
	//double v_luz=0.5, n=0.5*tanh(ix-100)+1.5; // La tanh permite cambiar suavemente de medio (n1=1, n2=2);
	//double C = v_luz/n;
  return 0.3;
}

bool LatticeBoltzmann::nodo(int ix, int iy){
  return ix >= 50 and pow((ix-50),2)+pow((iy-100),2) >= pow(100,2);
}

//---------------- Funciones Globales --------

int main(void){
  LatticeBoltzmann Ondas;
    int t,tmax=1000;

  double rho0=0,Jx0=0,Jy0=0;

  std::cout << "set term jpeg enhanced" << std::endl;
  std::cout << "set output 'Imagen_espejo.jpg'" << std::endl;
  std::cout << "set pm3d map" << std::endl;
  std::cout << "set cbrange[-10:10]" << std::endl;
  std::cout << "set xrange[0:250]; set yrange[0:200]" << std::endl;

  //Inicie
  Ondas.Inicie(rho0,Jx0,Jy0);
  //Corra
  for(t=0;t<tmax;t++){
    Ondas.Colisione();
    Ondas.ImponerCampos(t);
    Ondas.Adveccione();
    //Ondas.Imprimase("Esferico.dat");
    //std::cout << "splot 'Esferico.dat'" << std::endl;
  }
  
  //Mostrar Resultado.
  Ondas.Imprimase("Esferico.dat");
  //Ondas.ImprimaUnaLinea("CorteCentralE.dat",t);
  std::cout << "set title 'Esferico'" << std::endl;
  std::cout << "splot 'Esferico.dat'" << std::endl;

  return 0;
}

