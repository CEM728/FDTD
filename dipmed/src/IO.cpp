
#include "IO.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "constants.hpp"

/****************** Constructor/Destructor ************************************/
IO::IO(const int _argc, const char **_argv)
{
     if(_argc < 3)
     {
          printf("\n");
          printf("Please provide an input file name and output directory.\n");
          printf("Example: ./program <input file with path> <output directory>\n");
          printf("\n");
          abort();
     }
     
     /************* Read the input file ***************************************/
     input_file = _argv[1];
     output_dir = _argv[2];
     printf("\n");
     read_input_file();
     copy_input_file();
     screenout = nsteps/10;
     fileout = step;  
}
          
/****************** Member functions ******************************************/
void IO::copy_input_file()
/*
     Reference : http://hubpages.com/hub/File-Copy-Program-in-C-Language
*/
{
     char ch; 
     std::string output_file = output_dir + "input.txt";
     FILE *inF = fopen(input_file.c_str(),"r");
     FILE *ouF = fopen(output_file.c_str(),"w");
     if(inF == NULL) perror(input_file.c_str()), abort();
     if(ouF == NULL) perror(output_file.c_str()), abort();
     
     while(1)  
     {  
          ch = getc(inF);  
          if(ch==EOF) break;  
          else putc(ch,ouF);  
     }
     
     fclose(inF);
     fclose(ouF);
     printf("Input file copied to %s\n",output_dir.c_str());
     printf("\n");
}

/******************************************************************************/
void IO::read_input_file()
{
     FILE *f = fopen(input_file.c_str(),"r");
     if(f==NULL)
     {
          printf("Can't open %s\n",input_file.c_str());
          abort();
     }
     
     /************* Read the file *********************************************/
     printf("Reading %s\n",input_file.c_str());
     int nparams = 10;
     int results[nparams];
     
     int i = 0;
     results[i++] = fscanf(f,"nsteps = %i\n", &nsteps);
     if(nsteps <= 1) nsteps = 2;
     results[i++] = fscanf(f,"step = %i\n", &step);
     results[i++] = fscanf(f,"ncell = %i\n", &ncell);
     results[i++] = fscanf(f,"m2start = %i\n", &m2start);
     results[i++] = fscanf(f,"m2stop = %i\n", &m2stop);
     results[i++] = fscanf(f,"epsilon = %lf\n", &epsilon);
     results[i++] = fscanf(f,"dx = %lf\n", &dx);
     dt = time_scale*dx/co;
     results[i++] = fscanf(f,"t0 = %lf\n", &t0);
     results[i++] = fscanf(f,"spread = %lf\n", &spread);
     results[i++] = fscanf(f,"freq_in = %lf\n", &freq_in);
     assert(i-1-nparams);
     
     for(int i=nparams;i--;) 
     {
          if(results[i]!=1)
          {
               printf("Input file is incomplete.\n");
               printf("\n");
               printf("Example:\n");
               printf("nsteps = 1000\n");
               printf("step = 10\n");
               printf("ncell = 400\n");
               printf("m2start = 200\n");
               printf("m2stop = 400\n");
               printf("epsilon = 4.0\n");
               printf("dx = 0.01");
               printf("t0 = 80.0\n");
               printf("spread = 40.0\n");
               printf("freq_in = 2.0e9\n");
               printf("\n");
               abort();
          }
     }
     
     /*************************************************************************/
     fclose(f);
     
     /************* Print what was read ***************************************/
     printf("\n");
     printf("Input parameters\n");
     printf("nsteps = %d\n",nsteps);
     printf("step = %d\n",step);
     printf("ncell = %d\n",ncell);
     printf("m2start = %d\n",m2start);
     printf("m2stop = %d\n",m2stop);
     printf("epsilon = %f\n",epsilon);
     printf("dx = %f\n",dx);
     printf("dt = %e (time scale %f)\n",dt,time_scale);
     printf("t0 = %f\n",t0);
     printf("spread = %f\n",spread);
     printf("freq_in = %f\n",freq_in);
     printf("\n");
}

/******************************************************************************/
void IO::write_field_to_file(const int n, const double *ex, const double *hy)
{
     char filename[200];
     sprintf(filename,"./output/out_%.06d.dat",n);
     std::ofstream fp;
     fp.open(filename,std::ios::out);
     fp.precision(6);
     // Loop goes from 0 to KE-1 (excluded) because hy[KE-1] requires
     // ex[KE] that is undefined.
     for(int k=0; k < ncell-1; k++)
     {   // Typecast to (float) to avoid writing numbers with large exponents in the file
          fp << (float)k     << "\t" 
             << (float)ex[k] << "\t"
             << (float)hy[k] << std::endl;
     }      
     fp.close();
}
/****************** End of file ***********************************************/