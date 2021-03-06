#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "system.hpp"

#define epsz 8.8e-12

/******************************************************************************/
void add_a_dielectric_cylinder(double **ga, double **gb, const double epsilon,
                                const double sigma, const double radius, 
                                 const int ic, const int jc, const int ia, 
                                  const int ib, const int ja, const int jb, 
                                   const double dt)
{
     for(int j = ja; j < jb; j++)
          for(int i = ia; i < ib; i++)
          {
               double xdist = (ic - i);
               double ydist = (jc - j);
               double dist  = sqrt(xdist*xdist + ydist*ydist);
               if(dist <= radius)
               {
                    ga[i][j] = 1.0/(epsilon + (sigma*dt/epsz));
                    gb[i][j] = sigma*dt/epsz;
               }
          }
}

/******************************************************************************/
void add_a_dielectric_cell(double **ga, double **gb, const double epsilon,
                            const double sigma, const int ic, const int jc,
                              const double dt)
{
     ga[ic][jc] = 1.0/(epsilon + (sigma*dt/epsz));
     gb[ic][jc] = sigma*dt/epsz;
}

/******************************************************************************/
void Write(const double **array, const int IE, const int JE, const int n, 
            const char *tag)
{
     char filename[200];
     sprintf(filename,"output/%s_%06d.dat",tag,n);
     FILE *fp = fopen(filename,"w");
     for(int j=0; j < JE; j++ ) 
     {
          for(int i=0; i < IE; i++ ) 
          {
               fprintf(fp,"%6.3f ",array[i][j]);
          }
          fprintf(fp," \n");
     }
     fclose(fp);
}

/******************************************************************************/
int main(int argc, char *argv[])
{    
     const time_t start = time(NULL);
     
     /************* Get the number of steps to calculate **********************/
     int result = 0;
     int nsteps = 1;
     if(argc > 1) result = sscanf(argv[1],"%d", &nsteps);
     else
     {
          printf("nsteps --> ");
          result = scanf("%d", &nsteps);
     }
     
     /*************************************************************************/
     if(result != 1) 
     {
          printf("nsteps should be an integer.\n");
          abort();
     }
     else printf("nsteps = %d\n",nsteps);
     
     /************* Simulation parameters *************************************/
     const int IE = 1000;
     const int JE = 1000;
     const int npml = 8;
     const int outper = 10;

     const int shift[2] = {0,0};
     const int ic = (int) (IE/2 - shift[0]);
     const int jc = (int) (JE/2 - shift[1]);
     
     const int ntfsf = npml + 1;
//      const int ntfsf = 100;
     const int ia = ntfsf;
     const int ib = IE - ia - 1;
     const int ja = ntfsf;
     const int jb = JE - ja - 1;
     
     const double dx = .01; /* Cell size */
     const double dt = dx/6e8; /* Time steps */

//      const double t0 = 40.0;
//      const double spread = 12.0;
     
//      const double radius = 0.5;
     const double epsilon = 5.0;
     const double sigma = 0.0;
     
     /*************************************************************************/
     /************* Field arrays **********************************************/
     /*************************************************************************/
     printf("Array allocation ");fflush(stdout);
     double **Ez; allocate_2D(&Ez,IE,JE);
     double **ga; allocate_2D(&ga,IE,JE);
     double **gb; allocate_2D(&gb,IE,JE);     
     double **Dz; allocate_2D(&Dz,IE,JE);
     double **Iz; allocate_2D(&Iz,IE,JE);
     double **Hx; allocate_2D(&Hx,IE,JE);
     double **Hy;  allocate_2D(&Hy,IE,JE);
     double **ihx; allocate_2D(&ihx,IE,JE);
     double **ihy; allocate_2D(&ihy,IE,JE);

     /************* Initialize the arrays *************************************/
     for(int j=0; j < JE; j++ ) 
          for(int i=0; i < IE; i++ ) 
          {
               Ez[i][j] = 0.0;
               Dz[i][j] = 0.0;
               Iz[i][j] = 0.0;
               Hx[i][j] = 0.0;
               Hy[i][j] = 0.0;
               ihx[i][j] = 0.0;
               ihy[i][j] = 0.0;
               ga[i][j] = 1.0;
               gb[i][j] = 0.0;
          }
          
     Write((const double **)Ez,IE,JE,0,"Ez");
     printf(" => done\n"); fflush(stdout);
     
     /*************************************************************************/
     /************* Calculate the PML parameters ******************************/     
     /*************************************************************************/
     printf("Preparing the pml ");fflush(stdout);
     double gi2[IE];
     double gi3[IE];
     double fi1[IE];
     double fi2[IE];
     double fi3[IE];
     for(int i=0;i< IE; i++) 
     {
          gi2[i] = 1.0;
          gi3[i] = 1.0;
          fi1[i] = 0.0;
          fi2[i] = 1.0;
          fi3[i] = 1.0;
     }
     
     /*************************************************************************/
     for(int i=0;i<= npml; i++) 
     {
          double xnum = npml - i;
          double xd = npml;
          double xxn = xnum/xd;
          double xn = 0.25*pow(xxn,3.0);
//           printf(" %d %7.4f %7.4f \n",i,xxn,xn);
          gi2[i] = 1.0/(1.0+xn);
          gi2[IE-1-i] = 1.0/(1.0+xn);
          gi3[i] = (1.0 - xn)/(1.0 + xn);
          gi3[IE-i-1] = (1.0 - xn)/(1.0 + xn);
          xxn = (xnum-.5)/xd;
          xn = 0.25*pow(xxn,3.0);
          fi1[i] = xn;
          fi1[IE-2-i] = xn;
          fi2[i] = 1.0/(1.0+xn);
          fi2[IE-2-i] = 1.0/(1.0+xn);
          fi3[i] = (1.0 - xn)/(1.0 + xn);
          fi3[IE-2-i] = (1.0 - xn)/(1.0 + xn);
     }
     
     /*************************************************************************/
     double gj2[JE];
     double gj3[JE];
     double fj1[JE];
     double fj2[JE];
     double fj3[JE];
     for(int j=0;j< JE; j++) 
     {
          gj2[j] = 1.0;
          gj3[j] = 1.0;
          fj1[j] = 0.0;
          fj2[j] = 1.0;
          fj3[j] = 1.0;
     }

     /*************************************************************************/
     for(int j=0;j<= npml; j++) 
     {
          double xnum = npml - j;
          double xd = npml;
          double xxn = xnum/xd;
          double xn = 0.25*pow(xxn,3.0);
//           printf(" %d %7.4f %7.4f \n",i,xxn,xn);
          gj2[j] = 1.0/(1.0+xn);
          gj2[JE-1-j] = 1.0/(1.0+xn);
          gj3[j] = (1.0 - xn)/(1.0 + xn);
          gj3[JE-j-1] = (1.0 - xn)/(1.0 + xn);
          xxn = (xnum-.5)/xd;
          xn = 0.25*pow(xxn,3.0);
          fj1[j] = xn;
          fj1[JE-2-j] = xn;
          fj2[j] = 1.0/(1.0+xn);
          fj2[JE-2-j] = 1.0/(1.0+xn);
          fj3[j] = (1.0 - xn)/(1.0 + xn);
          fj3[JE-2-j] = (1.0 - xn)/(1.0 + xn);
     }

//      printf("gi + fi \n");
//      for (i=0; i< IE; i++) 
//      {
//           printf( "%2d %5.2f %5.2f \n",
//           i,gi2[i],gi3[i]),
//           printf( " %5.2f %5.2f %5.2f \n ",
//           fi1[i],fi2[i],fi3[i]);
//      }
// 
//      printf("gj + fj \n");
//      for (j=0; j< JE; j++) 
//      {
//           printf( "%2d %5.2f %5.2f \n",
//           j,gj2[j],gj3[j]),
//           printf( " %5.2f %5.2f %5.2f \n ",
//           fj1[j],fj2[j],fj3[j]);
//      }

     printf("=> done\n");fflush(stdout);

     /*************************************************************************/
     /************* Arrays for the incident wave source ***********************/     
     /*************************************************************************/
     double Ez_inc[JE];
     double Hx_inc[JE];
     for(int j=0; j < JE; j++)
     {
          Ez_inc[j] = 0.0;
          Hx_inc[j] = 0.0;
     }
     
     double Ez_inc_low_m1 = 0.0;
     double Ez_inc_low_m2 = 0.0;
     double Ez_inc_high_m1 = 0.0;
     double Ez_inc_high_m2 = 0.0;
     
     /************* Random scatterers *****************************************/
     printf("Preparing medium ");fflush(stdout);
//      int xmax = 340;
//      int ymax = 20;
//      int N = 3000;
//      for(int n = N; n--;)
//      {
//           int x = (double)rand()/RAND_MAX*xmax + ic - xmax/2;
//           int y = (double)rand()/RAND_MAX*ymax + jc - ymax/2 - 50;
//           add_a_dielectric_cylinder(ga,gb,epsilon,sigma,radius,x,y,ia,ib,ja,jb,dt);
//      }
//      
//      xmax = 20;
//      ymax = 20;
//      N = xmax*ymax;
//      for(int n = N; n--;)
//      {
//           int x = (double)rand()/RAND_MAX*xmax + ic - xmax/2;
//           int y = (double)rand()/RAND_MAX*ymax + jc - ymax/2 - 60;
//           add_a_dielectric_cylinder(ga,gb,epsilon,sigma,radius,x,y,ia,ib,ja,jb,dt);
//      }
          
     /************* Uniform scatterers ****************************************/
     int xmax = 180;
     int ymax = 180;
     for(int j = ymax; j--;)
          for(int i = xmax; i--;)
          {
               int x = i + ic - xmax/2;
               int y = j + jc - ymax/2 - 50;
               add_a_dielectric_cell(ga,gb,epsilon,sigma,x,y,dt);
          }
     
//      xmax = 20;
//      ymax = 20;
//      for(int j = ymax; j--;)
//           for(int i = xmax; i--;)
//           {
//                int x = i + ic - xmax/2;
//                int y = j + jc - ymax/2 - 60;
//                add_a_dielectric_cylinder(ga,gb,epsilon,sigma,radius,x,y,ia,ib,ja,jb,dt);
//           }
     
     Write((const double **)ga,IE,JE,0,"Ga");
     printf(" => done and profile written to file.\n");fflush(stdout);
     
     /************* Loop over all time steps **********************************/
     printf("Main loop \n");fflush(stdout);
     int T = 0;
     for(int n=1; n <=nsteps ; n++) 
     {
          T += 1;
          if(T%10==0) printf("*"),fflush(stdout);
          if(T%(int)(nsteps/10)==0) printf(" %d \n",T),fflush(stdout);

          /******** Update Ez_inc *********************************************/
          for(int j=1; j < JE; j++)
               Ez_inc[j] += 0.5*(Hx_inc[j-1] - Hx_inc[j]);
          
          Ez_inc[0]     = Ez_inc_low_m2;
          Ez_inc_low_m2 = Ez_inc_low_m1;
          Ez_inc_low_m1 = Ez_inc[1];
          
          Ez_inc[JE - 1] = Ez_inc_high_m2;
          Ez_inc_high_m2 = Ez_inc_high_m1;
          Ez_inc_high_m1 = Ez_inc[JE - 2];
          
          /******** Calculate the Dz field ************************************/
          for(int j=1; j < IE; j++ ) 
          {
               for(int i=1; i < IE; i++ ) 
               {
                    Dz[i][j] = gi3[i]*gj3[j]*Dz[i][j]
                    + gi2[i]*gj2[j]*.5*(Hy[i][j] - Hy[i-1][j]
                    - Hx[i][j] + Hx[i][j-1]) ;
               }
          }

          /******** Source ****************************************************/
          double pulse = sin(2.0*3.14159*1.5e9*T);
//           double pulse = exp(-0.5*pow( (T-t0)/spread,2.0));
//           Dz[ic][jc] = 5.0*pulse;
//           Dz[ia+20][jc] = 5.0*pulse;
//           Dz[ic + 50][jc + 50] = pulse;

          /******** Ez TFSF ***************************************************/
          Ez_inc[3] = pulse;
          for(int i = ia; i<= ib; i++)
          {
               Dz[i][ja] = Dz[i][ja] + 0.5*Hx_inc[ja - 1];
               Dz[i][jb] = Dz[i][jb] - 0.5*Hx_inc[jb];
          }
          
          /******** Calculate the Ez field ************************************/
          for(int j=1; j < JE-1; j++) 
          {
               for(int i=1; i < IE-1; i++) 
               {
                    Ez[i][j] = ga[i][j]*(Dz[i][j] - Iz[i][j]) ;
                    Iz[i][j] = Iz[i][j] + gb[i][j]*Ez[i][j] ;
               }
          }

          /******** Set the PEC at the simulation boundaries ******************/
          for(int j=0; j < JE-1; j++)
          {
               Ez[0][j] = 0.0;
               Ez[IE-1][j] = 0.0;
          }
          for(int i=0; i < IE-1; i++)
          {
               Ez[i][0] = 0.0;
               Ez[i][JE-1] = 0.0;
          }

          /******** Update Hx source ******************************************/
          for(int j = 0; j < JE-1; j++)
               Hx_inc[j] += 0.5*(Ez_inc[j] - Ez_inc[j + 1]);

          /******** Calculate the Hx field ************************************/
          for(int j=0; j < JE-1; j++) 
          {
               for(int i=0; i < IE; i++) 
               {
                    double curl_e = Ez[i][j] - Ez[i][j+1] ;
                    ihx[i][j] = ihx[i][j] + fi1[i]*curl_e ;
                    Hx[i][j] = fj3[j]*Hx[i][j]
                    + fj2[j]*.5*(curl_e + ihx[i][j]);
               }
          }

          /******** Incident Hx values ****************************************/
          for(int i = ia; i <= ib; i++)
          {
               Hx[i][ja-1] += 0.5*Ez_inc[ja];
               Hx[i][jb]   -= 0.5*Ez_inc[jb];
          }

          /******** Calculate the Hy field ************************************/
          for(int j=0; j <= JE-1; j++ ) 
          {
               for(int i=0; i < IE-1; i++ ) 
               {
                    double curl_e = Ez[i+1][j] - Ez[i][j];
                    ihy[i][j] = ihy[i][j] + fj1[j]*curl_e ;
                    Hy[i][j] = fi3[i]*Hy[i][j]
                    + fi2[i]*.5*(curl_e + ihy[i][j]);
               }
          }
          
          /******** Incident Hy values ****************************************/
          for(int j = ja; j <= jb; j++)
          {
               Hy[ia - 1][j] -= 0.5*Ez_inc[j];
               Hy[ib][j]     += 0.5*Ez_inc[j];
          }
          
          /******** Write Ez to file ******************************************/
          if(n%outper==0) Write((const double **)Ez,IE,JE,n,"Ez");
     }
     
     /************* Print message *********************************************/
     printf("Done\n ");
     
     /************* Free memory ***********************************************/
     delete_2D(Ez,IE);
     delete_2D(ga,IE);
     delete_2D(gb,IE);
     delete_2D(Dz,IE);
     delete_2D(Iz,IE);
     delete_2D(Hx,IE);
     delete_2D(Hy,IE);
     delete_2D(ihx,IE);
     delete_2D(ihy,IE);
     
     /*************************************************************************/
     ShowRunTime(start,time(NULL));
}

/****************** End of file ***********************************************/
