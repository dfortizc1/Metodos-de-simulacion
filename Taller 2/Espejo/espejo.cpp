#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;

const int Lx=600;
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
  int V[2][Q];  //V[0][i]=Vix     V[1][i]=Viy
  double f[Lx][Ly][Q],fnew[Lx][Ly][Q]; // f[ix][iy][i]
public:
  LatticeBoltzmann(void);
  double rho(int ix,int iy,bool UseNew);
  double Jx(int ix,int iy,bool UseNew);
  double Jy(int ix,int iy,bool UseNew);
  double feq(int ix,int iy, int i,double rho0,double Jx0,double Jy0);
  void Inicie(double rho0,double Jx0,double Jy0);
  void ImponerCampos(int ix,int iy,double & rho0,double & Jx0,double & Jy0,int t);
  void Colisione(int t);
  void Adveccione(void);
  void Imprimase(char const * NombreArchivo,int t);
  void ImprimaUnaLinea(char const * NombreArchivo,int t);
  
  //Velocidad de la onda.
  double Ccelda(int ix, int iy);
  
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
  else suma+=V[1][i]*f[ix][iy][i];
  return suma;
}
double LatticeBoltzmann::feq(int ix, int iy, int i,double rho0,double Jx0,double Jy0){
  double C=Ccelda(ix, iy), C2=C*C, TresC2=3*C2, AUX0=1-TresC2*(1-W0);
  if(i==0)
    return AUX0*rho0;
  else
    return w[i]*(TresC2*rho0+3*(V[0][i]*Jx0+V[1][i]*Jy0));
}
void LatticeBoltzmann::ImponerCampos(int ix,int iy,double & rho0,double & Jx0,double & Jy0,int t){
  double A=10,lambda=10,omega=2*M_PI*Ccelda(ix, iy)/lambda;
  if(ix==0)
    rho0=A*sin(omega*t);
}

void LatticeBoltzmann::Inicie(double rho0,double Jx0,double Jy0){
  int ix,iy,i;
  for(ix=0;ix<Lx;ix++)
    for(iy=0;iy<Ly;iy++)
      for(i=0;i<Q;i++)
		f[ix][iy][i]=feq(ix, iy, i,rho0,Jx0,Jy0);
}
void LatticeBoltzmann::Colisione(int t){ //de f a fnew
  int ix,iy,i; double rho0,Jx0,Jy0;
  for(ix=0;ix<Lx;ix++)
    for(iy=0;iy<Ly;iy++){ //Para cada celda
      rho0=rho(ix,iy,false);  Jx0=Jx(ix,iy,false);  Jy0=Jy(ix,iy,false); //Calculo campos
      //if(ix == 0 or iy == 0){rho0=0.0, Jx0=0.0, Jy0=0.0;}
	ImponerCampos(ix,iy,rho0,Jx0,Jy0,t);     
	for(i=0;i<Q;i++) //para cada dirección
	  fnew[ix][iy][i]=UmUtau*f[ix][iy][i]+Utau*feq(ix, iy, i,rho0,Jx0,Jy0); //evoluciono
    }
}
void LatticeBoltzmann::Adveccione(void){ //de fnew a f
  int ix,iy,i;
  for(iy=0;iy<Ly;iy++)
	for(ix=0;ix<Lx;ix++)
      for(i=0;i<Q;i++)
	f[(ix+V[0][i]+Lx)%Lx][(iy-V[1][i]+Ly)%Ly][i]=fnew[ix][iy][i];
}
void LatticeBoltzmann::Imprimase(char const * NombreArchivo,int t){
  ofstream MiArchivo(NombreArchivo); double rho0,Jx0,Jy0;
  //for(int ix=0;ix<Lx;ix++){
  for(int ix=0;ix<Lx/2.0;ix++){
    for(int iy=0;iy<Ly;iy++){
      rho0=rho(ix,iy,true);   Jx0=Jx(ix,iy,false);  Jy0=Jy(ix,iy,false);
      ImponerCampos(ix,iy,rho0,Jx0,Jy0,t);
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
    ImponerCampos(ix,iy,rho0,Jx0,Jy0,t);
    MiArchivo << ix << " " << rho0 << endl;
  }
  MiArchivo.close();
}

double LatticeBoltzmann::Ccelda(int ix, int iy){
 double v_luz=0.5, x=50+sqrt(100*100-(iy-100)*(iy-100)), n=0.5*tanh(ix-x)+1.5;
	double C = v_luz/n;
	return C;
	}

//---------------- Funciones Globales --------

int main(void){
  LatticeBoltzmann Ondas;
   int t,tmax=600;

  double rho0=0,Jx0=0,Jy0=0;

  //Inicie
  Ondas.Inicie(rho0,Jx0,Jy0);
  //Corra
  for(t=0;t<tmax;t++){
    Ondas.Colisione(t);
    Ondas.Adveccione();
  }
  
  //Mostrar Resultado.
  Ondas.Imprimase("Espejo.dat",t);
  Ondas.ImprimaUnaLinea("CorteCentral.dat",t);

  return 0;
}



