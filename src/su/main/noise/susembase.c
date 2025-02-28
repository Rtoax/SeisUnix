/* Copyright (c) Colorado School of Mines, 2022.*/
/* All rights reserved.                       */

/* SUSEMBASE: $Revision: 1.1 $ ; $Date: 2023/03/02 23:05:59 $      */

#include <stdio.h>
#include <string.h>

#include "su.h"
#include "segy.h"
#include "headcase.h"

segy tr;
segy trd;

/*********************** self documentation **********************/
char *sdoc[] = {
"                                                                            ",
" SUSEMBASE - Semblance Based Structure-Oriented Interpolate and Smooth.     ",
"                                                                            ",
" This program performs semblance scanning to derive (apparent) dips in the  ",
" direction of cdp numbers (or other key values). It then uses those dips to ",
" align trace sample values while performing interpolation and/or smoothing. ",
"                                                                            ",
" susembase    <in.su >out.su  (other parameters)                            ",
"                                                                            ",
" Parameter overview:                                                        ",
"                                                                            ",
" lockey=cdp   Trace key that indicates trace location. Input traces must be ",
"              in increasing-OR-EQUAL order of these values. This key value  ",
"              is reset on output. See outputinc and outputalign parameters. ",
"        Note: The input locations must be in increasing-OR-EQUAL order.     ",
"              So, traces can have duplicate lockeys provided the numrolling ",
"              parameter is set large enough. Whether these duplicates have  ",
"              serious effects on the output traces varies with situation.   ",
"                                                                            ",
" groupkey=    Trace key that indicates the end of a set of lockey values.   ",  
"              If specified, traces must be ordered by lockey and groupkey.  ",  
"              If a 3D stacked dataset is input, this parameter should often ",  
"              be either igi or igc, depending on which direction is being   ",  
"              processed in this run (see program subincvs).                 ",  
"        Note: The keys for susort are listed in reverse order with respect  ",  
"              to lockey and groupkey here.                                  ",  
"                                                                            ",
" outputinc=1.0    Increment for output locations. Example: if a cdp stack   ",
"                  is input and cdp numbers increment by 1 then setting this ",
"                  parameter to 0.5 will output about twice as many traces.  ",
"            Note: Since lockey is (almost always) integer, it cannot hold   ",
"                  decimal fractions, so if this outputinc parameter is      ",
"                  between 0 and 1, the lockey output values are inversed:   ",
"                   (input_values - outputalign) / outputinc + outputshift   ",
"                  If outputinc is greater than 1 then the lockey output     ",
"                  values are:    (input_values - outputalign) + outputshift ",
"                                                                            ",
" outputalign=0.0  Alignment for output locations. Example: if a cdp stack   ",
"                  is input and cdp numbers increment by 1 and outputinc=1.0 ",
"                  then setting this parameter to 0.25 will create output    ",
"                  traces at input cdp numbers plus 0.25                     ",
"                                                                            ",
" outputshift=0.0  Add this value to output location numbers. If lockey is   ",
"                  cdp and first input cdp number is small, traces may be    ",
"                  extrapolated to negative cdp numbers. This may be an      ",
"                  issue for some situations, so you can add this constant.  ",
"                                                                            ",
" locmaxscan=3 Maximum Relative lockey to include in scanning for dips.      ",
"              Must be greater than or equal to 0. This program scans traces ",
"              from -locmaxscan to locmaxscan (inclusive) at each location.  ",
"           =0 Do not scan for dips. You must specify parameter locmaxsum    ",
"              and you cannot specify scanstep, scanmax, scanmin, dipext.    ",
"                                                                            ",
" scanstep=0.5   Time Step to Scan (milliseconds per 1 lockey unit).         ",
" scanmax =12.0  Maximum Time to Scan (milliseconds per 1 lockey unit).      ",
" scanmin =-scanmax  Minimum Time to Scan (milliseconds per 1 lockey unit).  ",
"                                                                            ",
" dipext=12. Dip Extender (milliseconds).                                    ",
"            At each location the semblance scanning produces an estimate    ",
"            of the dip at each sample time. This parameter performs a       ",
"            weighted-average of those estimated dips symmetrically from     ",
"            -dipext to +dipext. The weights used are the sum of the         ",
"            squared amplitudes of seismic values along the estimated dip    ",
"            in the locmaxscan range of input traces. In cruder terms: the   ",
"            power along the dip value chozen by the semblance scanning.     ",
"            Effectively, this causes the dip values from large horizons     ",
"            to be trusted more and extended up and down nearby times.       ",
"                                                                            ",
" dipuse=1  Estimate and use dips at input locations.                        ",
"       =0  Estimate and use dips at output locations.                       ",
"                                                                            ",
" locmaxsum=locmaxscan  Maximum Relative lockey to include in sumopt options.",
"                       Must be greater than 0. This program uses the input  ",
"                       traces from -locmaxsum to locmaxsum (inclusive) at   ",
"                       each output location.                                ",
"                                                                            ",
" sumopt=stack All options here only include input traces within -locmaxsum  ",
"              to +locmaxsum range (inclusive) around each output location.  ",
"              This default just adds them all together and divides by total.",
"              Note that correct dips from the semblance scanning will cause ",
"              these traces to be locally flat during stacking and therefore ",
"              the output will not have as much smearing as traces stacked   ",
"              without dip compensation.                                     ",
"       =sum   Just like option stack but do not divide by total.            ",
"              (This option is intended to help with parameter setting).     ",
"       =one   Find the input trace with lockey value nearest the output     ",
"              location, apply the dips, and output it. Effectively, this    ",
"              is interpolation using just the nearest trace.                ",
"       =two   Find the two input traces with lockey value nearest output,   ",
"              apply the dips, sum both traces and divide by 2, and output.  ",
"       =twolinear Find the two input traces with lockey values nearest      ",
"                  output, apply the dips, stack them with linear weight     ",
"                  based on their locations, and output. (Often one of the   ",
"                  two traces will get all the weight, of course).           ",
"       =dadstack  Detect-Amplitude-Differences. You should expect to have   ",
"                  to do considerable testing of the other dad parameters    ",
"                  for this option. Precise setting of the dad parameters    ",
"                  may result in the output traces being sharper at rapid    ",
"                  amplitude changes (such as locations where horizons start ",
"                  or terminate. These dad options search for rapid changes  ",
"                  in amplitude ACROSS LOCATIONS at each sample time and     ",
"                  only stacks the sample values from -locmaxsum TO the      ",
"                  detected location -or- FROM the detected location to the  ",
"                  +locmaxsum location. Which side is stacked depends on     ",
"                  WHERE detected location is relative to output location.   ",
"             ***  If a rapid change in amplitude is not detected at this    ",
"                  sample time then ALL samples are stacked.                 ",
"       =dadsum  Just like option dadstack but do not divide by total.       ",
"                (This option is intended to help with parameter setting).   ",
"                                                                            ",
" dadscale=1.0 Detect-Amplitude-Differences scaler.                          ",
"             This parameter can only be set if sumopt=dadstack or dadsum.   ",
"             When a rapid change in amplitude is detected this parameter    ",
"             causes the range of locations stacked at this sample to        ",
"             increase or decrease.                                          ",
"       Note: Usual range for this value is 0.9 to 1.1                       ",
"                                                                            ",
" dadnoise=0.0  Detect-Amplitude-Differences Noise Absolute Amplitude.       ",
"              This parameter can only be set if sumopt=dadstack or dadsum.  ",
"              If the average absolute amplitude at the sample time across   ",
"              the range of input traces is less than this value, this       ",
"              sample time is not considered to have detected a rapid change ",
"              in amplitude (so all locations are stacked for output sample).",
"                                                                            ",
" dadext=12.   Detect-Amplitude-Differences extender (milliseconds).         ",
"              This parameter can only be set if sumopt=dadstack or dadsum.  ",
"              At each output location the amplitude difference detector     ",
"              estimates if-and-where a rapid difference is located in the   ",
"              locmaxsum range of input traces. This parameter performs a    ",
"              weighted-average of those estimated rapid difference locations",
"              symmetrically in time from -dadext to +dadext. The weights    ",
"              used are the sum of absolute amplitudes of the seismic values ",
"              at the same samples in the locmaxsum range of input traces.   ",
"              Effectively, this means the detected-amplitude-difference     ",
"              locations at large horizons are trusted more, and thus they   ",
"              are extended up and down in time near those larger horizons.  ",
"                                                                            ",
" outdip=   Do not output this file.                                         ",
"       =(file name) Output the dip values chosen by the semblance scanning  ",
"           into trace samples in this file. The trace header values are     ",
"           copied from the seismic traces. If dipuse=1 then they are the    ",
"           input headers, if dipuse=0 then they are the output headers.     ",
"     Note: These dips are after dipext processing. But even with dipext=0   ",
"           they will not have scanstep granularity because the scanned      ",
"           maximum semblance dip undergoes a 3-point interpolation to get   ",
"           a better approximation of the dip at the true peak in semblance. ",
"     Note: It is technically more correct to call the dips made and used    ",
"           by this program APPARANT dips. The dip values are put into the   ",
"           trace samples and are in milliseconds per 1 location unit.       ",
"           That is, if the lockey=cdp then the dips are ms per cdp. There   ",
"           is no dip azimuth output. The dips are in the direction from one ",
"           location to next (if lockey=cdp this is one cdp toward next cdp).",
"                                                                            ",
" outpow=   Do not output this file.                                         ",
"       =(file name) Output the average power along the dip direction chosen ",
"           by the semblance scanning. (Same header values as for outdip).   ",
"                                                                            ",
" outsem=   Do not output this file.                                         ",
"       =(file name) Output the semblance value computed and chosen by the   ",
"           semblance scanning. (Same header values as for outdip).          ",
"                                                                            ",
" indip=    Name of trace file which contains (apparant) dip values.         ",
"           Dip scanning is disabled if you use this parameter. Thus you     ",
"           cannot specify parameters scanstep, scanmax, scanmin, dipext,    ",
"           and locmaxscan - and you must specify parameter locmaxsum.       ",
"           This file is usually initially made by specifying outdip above.  ",
"           If dipuse=1 then this file must have the same order as the       ",
"           order as the seismic traces you are inputting in this run.       ",
"           If dipuse=0 then this file must have the same INPUT order as     ",
"           the OUTPUT order of this run.                                    ",
"     Note: The intention here is to allow you to output dips from one run   ",
"           of this program and use them in a second run of this program.    ",
"           The first run can input smoothed, denoised seismic to get dips,  ",
"           and the second run can input the un-smoothed, noisy seismic and  ",
"           use the dips from the first run. As well, you can also actually  ",
"           smooth and denoise the DIP VALUES in this file before input here.",
"                                                                            ",
"                                                                            ",
" bypass=0  Treat all output locations the same.                             ",
"       =1  If an input trace exists exactly at the output location, COPY    ",
"           the input trace without any change except to reset the lockey    ",
"           to the output location value. Process all other output locations ",
"           as indicated by the other options you have specified.            ",
"       =-1 Only process and output traces where an input trace is exactly   ",
"           on an output location. Typically you use this option when you    ",
"           just want to smooth the input, without creating any more output  ",
"           locations (so set outputalign=0 and usually set outputinc=1).    ",
"  CAUTION: Options 1 and -1 only retain the first trace with duplicate      ",
"           input location value. For instance if lockey=offset and          ",
"           groupkey=cdp then only the first trace with the same offset      ",
"           within cdps will be copied or processed to output.               ",
"                                                                            ",
" numrolling=2*locmaxscan+2*locmaxsum+5. This is maximum number of traces    ",
"              to store internally. Traces are read-in as needed to keep the ",
"              output locations centred in this storage (a rolling buffer).  ",
"          *** For certain setups, this default is too large. For instance,  ",
"              when lockey=offset and groupkey=cdp this value can be set to  ",
"              the maximum cdp fold (but must always be set greater than 4). ",
"          *** For certain setups, this default is too small. For instance,  ",
"              when duplicate input lockey values exist.                     ",
"        Note: This program expects input locations to have approximately    ",
"              the same interval between them and to be distributed nicely.  ",
"              The input locations must be in increasing-OR-EQUAL order.     ",
"              So, traces can have duplicate lockey values provided this     ",
"              parameter is set large enough. Whether these duplicates have  ",
"              serious effects on the output traces varies with situation.   ",
"                                                                            ",
" Note: This program enforces non-hierarchical parameter error checking.     ",
"       This means if you disable a main option, all parameters related      ",
"       to it must be removed from the command line. That is, for novice     ",
"       users this program is sufficiently complex that it refuses to guess  ",
"       which parameter was specified in error.                              ",
"                                                                            ",
" ------------------------------------------------------------------------   ", 
"                                                                            ",
NULL};

/* Credits:                                                                  */
/* Author: Andre Latour, Feb 2023                                            */ 
/*                                                                           */
/* Trace key usually involved: cdp   (user options: maybe igi,igc others)    */
/*                                                                           */
/**************** end self doc ***********************************************/

void scanit (segy **atrace, int *aloc, int numrolling, double scanloc, 
            double locminscan, double locmaxscan, float smin, float sstep, int jsmax, 
            float *adip, float *apow, float *asem, float *awrk, float *awrq, 
            int *arng, int **ifori, float **afori, int lendip, int *numout) ;

void wsmooth (float *val, float *wgt, int len, int minsd, int maxsd, float *sval) ;

int main(int argc, char **argv) {

  cwp_String lockey=NULL;
  int lockase=-1;

  cwp_String groupkey=NULL;
  int groupkase=-1;
  int usegroup=0;
  double thisgroup=-1.e30;
  int newgroup=0;

  segy **rolltrace=NULL; 
  segy *onetrace=NULL; 
  int    *rollloc=NULL;
  int    *rolldun=NULL;
  float  **rolldip=NULL;
  float  **jsumdat=NULL;
  float  *onedip=NULL;
  float  *onepow=NULL;
  float  *onepw2=NULL;
  float  *onetmp=NULL;
  float  *onetm2=NULL;
  float  *onesem=NULL;
  float  *onesm2=NULL;
  float  *onetim=NULL;
  float  *oneand=NULL;
  float  *dipwrk=NULL;
  float  *dipwrq=NULL;
  int    *locit=NULL;
  int    *arng=NULL;
  int   **ifori=NULL;
  float **afori=NULL;

  int lendip=0;
  int nproct=0;
  int mproct=0;
  int numout=0;
  int icycle=1;
  int iroll=1;
  int dipuse=1;
  int i=0;
  int j=0;
  int k=0;
  int klow=0;
  double diffk=1.e30;
  int jsum=0;
  double outputloc = 0.0;
  double outputinc = 1.0;
  double outputcni = 1.0;
  double outputalign = 0.0;
  double outputshift = 0.0;
  double locminscan = -3.0;
  double locmaxscan = 3.0;
  double locminsum  = -3.0;
  double locmaxsum  = 3.0;
  double locprev    = 0.0;
  int    locdups    = 0;
  int numrolling = 5;                                                                    
  int iend=0;
  int maxstoredloc=0;
  float dt=0.;
  float sstep=1.0;
  float smin=-8.;
  float smax=8.;
  float jsmax=0;
  float dipext=12.;
  int   minsd=-1;
  int   maxsd=1;

  float dadext=12.;
  int   mindad=-1;
  int   maxdad=1;

  cwp_String sumopt=NULL;
  int isumopt=3;
  int ilinear=0;
  int   jnear=-1;
  double diffj=1.e30;
  double diffj2=1.e30;
  int jsumn=0;
  int jsumn2=0;
  double centloc=0.0;
  int jfirst=0;
  int jlast=0;
  float dadscale=1.0;
  float ratio=0.0;
  float dadnoise=0.0;

  cwp_String dname=NULL; 

  int indip = 0;
  FILE *fpindip=NULL;      
  int ksum=0;

  int outdip = 0;
  FILE *fpoutdip=NULL;      
  int outpow = 0;
  FILE *fpoutpow=NULL;      
  int outsem = 0;
  FILE *fpoutsem=NULL;      

  int bypass=0;
  int iput=1;

/* Initialize */

  initargs(argc, argv);
  requestdoc(1);

  if(countparval("lockey") > 0) {
    getparstring("lockey", &lockey);
  }
  else {
    lockey = ealloc1(3,1);
    strcpy(lockey,"cdp");
  }
  lockase = GetCase(lockey);
  if(lockase<1) err("**** Error: Specified lockey name %s is not recognized.",lockey);

  if(countparval("groupkey") > 0) {
    getparstring("groupkey", &groupkey);
    groupkase = GetCase(groupkey);
    if(groupkase<1) err("**** Error: Specified groupkey name %s is not recognized.",groupkey);
    usegroup = 1;
  }

  if(!getpardouble("locmaxscan", &locmaxscan)) locmaxscan = 3.0;
  if(locmaxscan<0.0) err("error: locmaxscan must be greater than or equal to 0.");
  locminscan = 0.0 - locmaxscan;

  if(locmaxscan==0.0) {
    if(countparval("scanstep") > 0 || countparval("scanmax") > 0 || countparval("scanmin") > 0 ||
       countparval("dipext") > 0 || countparval("locmaxsum") < 1 ) {
       err("error: locmaxscan=0 does not allow parameters scanstep, scanmax, scanmin, dipext, and REQUIRES locmaxsum");
    }
  }

  if(!getpardouble("locmaxsum", &locmaxsum)) locmaxsum = locmaxscan;
  if(locmaxsum<=0.0) err("error: locmaxsum must be greater than 0.");
  locminsum = 0.0 - locmaxsum;
 
  if(!getparint("dipuse", &dipuse)) dipuse = 1;
  if(dipuse<0 || dipuse>1) err("error: dipuse parameter is out-of-range.");
 
  if(!getparint("bypass", &bypass)) bypass = 0;
  if(bypass<-1 || bypass>1) err("error: bypass parameter is out-of-range.");
 
  if(!getparfloat("dipext", &dipext)) dipext = 12.0;
  if(dipext<0.0) err("error: dipext must be equal to or greater than 0.0");

  if(!getparfloat("scanstep", &sstep)) sstep = 1.0;
  if(sstep<0.0) err("error: scanstep must be equal to or greater than 0.0");

  if(!getparfloat("scanmax", &smax)) smax = 12.0;

  if(!getparfloat("scanmin", &smin)) smin = 0.0 - smax;
  if(smax<smin) err("error: scanmax must be equal to or greater than scanmin");

/* The +5 is because ranges are inclusive, and for some float point tolerance. */
  if(!getparint("numrolling", &numrolling)) numrolling = locmaxscan-locminscan + locmaxsum-locminsum + 5; 
  if(numrolling<5) err("error: numrolling must be greater than 5");

  if(!getpardouble("outputinc", &outputinc)) outputinc = 1.0;
  if(outputinc<=0.0) err("error: outputinc must be greater than 0.0");
  if(outputinc<1.) outputcni = outputinc;
  else outputcni = 1.;

  if(!getpardouble("outputalign", &outputalign)) outputalign = 0.0;
  if(outputalign<0.0) err("error: outputalign must be equal to or greater than 0.0");
  outputalign = fmod(outputalign,outputinc);
 
  if(!getpardouble("outputshift", &outputshift)) outputshift = 0.0;

  isumopt = 3;
  ilinear = 0;
  if(countparval("sumopt") > 0) {
    isumopt = 0;
    getparstring("sumopt", &sumopt);
    if(strcmp(sumopt,"one") == 0) isumopt = 1;
    if(strcmp(sumopt,"two") == 0) isumopt = 2;
    if(strcmp(sumopt,"twolinear") == 0) {
      isumopt = 2;
      ilinear = 1;
    }
    if(strcmp(sumopt,"stack") == 0) isumopt = 3;
    if(strcmp(sumopt,"sum") == 0) isumopt = 4;
    if(strcmp(sumopt,"dadstack") == 0) isumopt = 5;
    if(strcmp(sumopt,"dadsum") == 0) isumopt = 6;
    if(isumopt<1) err("**** Error: Specified sumopt=%s option not recognized.",sumopt);
  }

  if(isumopt<5 && (countparval("dadscale") > 0 || countparval("dadext") > 0 ||
    countparval("dadnoise") > 0)) {
    err("error: only sumopt=dadstack or dadsum allows specifying parameters dadscale, dadext, and dadnoise");
  }

  if(!getparfloat("dadscale", &dadscale)) dadscale = 1.0;

  if(!getparfloat("dadext", &dadext)) dadext = 12.0;
  if(dadext<0.0) err("error: dadext must be equal to or greater than 0.0");

  if(!getparfloat("dadnoise", &dadnoise)) dadnoise = 0.0;
  if(dadnoise<0.0) err("error: dadnoise must be equal to or greater than 0.0");

  if(countparval("outdip") > 0) {
    getparstring("outdip", &dname);
    fpoutdip = efopen(dname, "w");
    if(fpoutdip==NULL) err("error: outdip file did not open correctly.");
    outdip = 1;
  }

  if(countparval("outpow") > 0) {
    getparstring("outpow", &dname);
    fpoutpow = efopen(dname, "w");
    if(fpoutpow==NULL) err("error: outpow file did not open correctly.");
    outpow = 1;
  }

  if(countparval("outsem") > 0) {
    getparstring("outsem", &dname);
    fpoutsem = efopen(dname, "w");
    if(fpoutsem==NULL) err("error: outsem file did not open correctly.");
    outsem = 1;
  }

/* Do this last to retain dname in case of error print */

  if(countparval("indip") > 0) { 
    if(countparval("scanstep") > 0 || countparval("scanmax") > 0 || countparval("scanmin") > 0 ||
       countparval("dipext") > 0 || countparval("locmaxscan") > 0 ) {
      err("error: indip file does not allow parameters scanstep, scanmax, scanmin, dipext, and locmaxscan");
    }
    getparstring("indip", &dname); 
    fpindip = efopen(dname, "r");
    if(fpindip==NULL) err("error: indip file did not open correctly.");
    indip = 1;
  }

/* -------------------------------------------------------------------------- */

  rollloc   = (int *)ealloc1(sizeof(int),numrolling);
  rolldun   = (int *)ealloc1(sizeof(int),numrolling);
  rolltrace = (segy **)ealloc1(sizeof(segy*),numrolling);
  onetrace  = (segy *)ealloc1(sizeof(segy),sizeof(char));
  rolldip   = (float **)ealloc1(sizeof(float*),numrolling);
  jsumdat   = (float **)ealloc1(sizeof(float*),numrolling);
  locit     = (int *)ealloc1(sizeof(int),numrolling);

  for (i=0; i<numrolling; i++) {
    rollloc[i]  = 0;
    rolldun[i]  = 0;
    rolltrace[i] = (segy *)ealloc1(sizeof(segy),sizeof(char));
  }

  arng = ealloc1int(numrolling);
  ifori = (int **)ealloc1(sizeof(int*),numrolling);
  afori = (float **)ealloc1(sizeof(float*),numrolling);

  checkpars(); 

/* ---Start processing traces.--------------------------------    */

  icycle = 1;
  outputloc = 0.0;
  iend   = 0;

/* loop over traces   */ 

  while(icycle==1) {
    if(gettr(&tr)) {

/* To get started, pretend that numgather locations were already stored but   */
/* were lower than the scan range for the first input location.               */

      if(nproct==0) {
        iroll  = 0;                                      
        dt     = ((double) tr.dt)/1000.0;
        sstep  = sstep / dt;
        smin   = smin  / dt;
        smax   = smax  / dt;
        maxsd  = (dipext+0.01) / dt;
        minsd  = 0 - maxsd;
        maxdad = (dadext+0.01) / dt;
        mindad = 0 - maxdad;
        jsmax  = 1 + (smax-smin)/sstep;
        dipwrk = ealloc1float(jsmax);
        dipwrq = ealloc1float(jsmax);
        lendip = tr.ns;
        onedip = ealloc1float(lendip);
        onepow = ealloc1float(lendip);
        onepw2 = ealloc1float(lendip);
        onetmp = ealloc1float(lendip);
        onetm2 = ealloc1float(lendip);
        onesem = ealloc1float(lendip);
        onesm2 = ealloc1float(lendip);
        onetim = ealloc1float(lendip);
        oneand = ealloc1float(lendip);
        for (i=0; i<numrolling; i++) {
          rolldip[i] = ealloc1float(lendip);
          jsumdat[i] = ealloc1float(lendip);
          ifori[i] = ealloc1int(jsmax);
          afori[i] = ealloc1float(jsmax);
/* Initialize location values to definitly out-of-range */
          rollloc[i] = fromhead(tr,lockase) + 3.0*(locminscan + locminsum) - i - 5;
        }

        maxstoredloc = rollloc[0];
        outputloc = fromhead(tr,lockase) * outputinc + outputalign + locminscan + locminsum -1.0; 
        locprev = fromhead(tr,lockase) - 1.;
      }
      nproct++;


      if(usegroup>0) {
        newgroup = 0;
        if(thisgroup != fromhead(tr,groupkase)) {
          thisgroup = fromhead(tr,groupkase);
          locprev = fromhead(tr,lockase) -1.;
          newgroup = 1;
          iend = 2*(locmaxscan + locmaxsum) + 2;
        }
      }

      if(fromhead(tr,lockase) <= locprev) {
        if(fromhead(tr,lockase) == locprev) locdups++;
        else err("input lockey %g is less than previous trace %g",fromhead(tr,lockase),locprev);
      }

      locprev = fromhead(tr,lockase);

    }
    else { /* no more traces to input */
      icycle = 0;
      iend = 2*(locmaxscan + locmaxsum) + 2;
    }

/* Note there might still be no traces output by scanit for outputloc since   */ 
/* stored locations might be, for example: 5,12,30 while the outputloc is 18. */ 

    while(outputloc < maxstoredloc - locmaxscan - locmaxsum + iend) {

      if(dipuse < 1) {  /* compute or input the dip at the output location ?  */
        if(indip<1) {  /* compute the dip ? */
          scanit (rolltrace,rollloc,numrolling,outputloc,
                  locminscan,locmaxscan,smin,sstep,jsmax,
                  onedip,onepw2,onesm2,dipwrk,dipwrq,
                  arng,ifori,afori,lendip,&numout);

/* Smooth the dip values in time (weighted by the average power along the     */
/* semblance-chozen dip). To call this smoothing is misleading since weights  */ 
/* actually cause dip values of strong horizons to expand up and down in time.*/
/* The expansion is the primary desire, not the smoothing.                    */

          wsmooth (onedip,onepw2,lendip,minsd,maxsd,onetim);
          for(i=0; i<lendip; i++) onedip[i] = onetim[i]; 

        } /* end of   if(indip<1) { */

        else { /* input dips? */
          ksum = 0; /* need to anticipate whether an output will be created */
          for (j=0; j<numrolling; j++) { 
            if(rollloc[j] >= outputloc+locminsum && rollloc[j] <= outputloc+locmaxsum) { 
              ksum = 1;
              break;
            }
          }
          if(ksum>0) { /* Note error-check uses outputcni, not outputinc. */
            if(!fgettr(fpindip,&trd))  
              err("next output location needs a trace from dipfile: %s   but get failed (near location=%g)",
                 dname,outputloc); 
            if(fabs((outputloc - outputalign) / outputcni + outputshift - fromhead(trd,lockase)) > 1.e-9) 
              err("lockey value for dipfile input trace and output location are not the same %g %g",
                   (outputloc - outputalign) / outputcni + outputshift,fromhead(trd,lockase)); 
            for (i=0; i<lendip; i++) onedip[i] = trd.data[i] / dt;
          }
        }

      } /* end of   if(dipuse < 1) { */

      jsum = 0;
      jnear = -1;
      diffj = 1.e30;
      diffj2 = 1.e30;
      centloc = 0.0;
      klow=-1;
      diffk = 1.e30;
      for (j=0; j<numrolling; j++) {

        if(rollloc[j] >= outputloc+locminsum && rollloc[j] <= outputloc+locmaxsum) { 

          if(dipuse > 0) { /* use the dip at each contributing trace ? */ 
            if(rolldun[j] < 1) { 
              scanit (rolltrace,rollloc,numrolling,(double)rollloc[j],
                      locminscan,locmaxscan,smin,sstep,jsmax,
                      onedip,onepow,onesem,dipwrk,dipwrq,
                      arng,ifori,afori,lendip,&numout);

              wsmooth (onedip,onepow,lendip,minsd,maxsd,rolldip[j]);

              rolldun[j] = 1;

              if(outdip>0 || outpow>0 || outsem>0) {
                memcpy(onetrace,rolltrace[j],sizeof(segy));
                if(outdip>0) {
                  for (i=0; i<lendip; i++) onetrace->data[i] = rolldip[j][i] * dt;
                  fputtr(fpoutdip,onetrace); 
                }
                if(outpow>0) {
                  for (i=0; i<lendip; i++) onetrace->data[i] = onepow[i];
                  fputtr(fpoutpow,onetrace); 
                }
                if(outsem>0) {
                  for (i=0; i<lendip; i++) onetrace->data[i] = onesem[i];
                  fputtr(fpoutsem,onetrace); 
                }
              }

            } /* end of  if(rolldun[j] < 1) { */
            for (i=0; i<lendip; i++) onetim[i] = (float) i + rolldip[j][i] * (rollloc[j] - outputloc);

          } 
          else { /* use the dip at the output location ? */
            for (i=0; i<lendip; i++) onetim[i] = (float) i + onedip[i] * (rollloc[j] - outputloc);
          }

/* Shift sample values by the time computed from dips and location difference.*/

          ints8r(lendip, 1.0, 0.0, rolltrace[j]->data, 0.0, 0.0, lendip, onetim, jsumdat[jsum]);

/* Keep track of 2 nearest traces (used later for some options).              */

          if(fabs((double)rollloc[j] - outputloc) < diffj) {
            diffj2 = diffj;
            jsumn2 = jsumn;
            jnear = j;
            jsumn = jsum;
            diffj = fabs((double)rollloc[j] - outputloc);
          }
          else if(fabs((double)rollloc[j] - outputloc) < diffj2) {
            diffj2 = fabs((double)rollloc[j] - outputloc);
            jsumn2 = jsum;
          }

          if(rollloc[j]<diffk) {
            diffk = rollloc[j];
            klow = jsum;
          }

          centloc += rollloc[j];
          locit[jsum] = rollloc[j];
          jsum++;

        } /* end of  if(rollloc[j] >= outputloc+locminsum && .... */

      } /* end of  for (j=0; j<numrolling; j++) { */

      if(jsum>0) {
        memcpy(onetrace,rolltrace[jnear],sizeof(segy)); /* Note outputcni next*/
        tohead(onetrace, lockase, (outputloc - outputalign) / outputcni + outputshift);

        iput = 1;
/* Just COPY the INPUT trace if it is at the output location exactly?         */ 
        if(bypass>0 && diffj<1.e-9) { 
          diffj = 1.e30;   /* does nothing. I just do not like empty bodies.  */
        }
/* Only PROCESS and output if an input exists at the output location exactly? */ 
        else if(bypass<0 && diffj>1.e-9) { 
          iput = 0;
        }
/* Otherwise, continue with the isumopt options.                              */ 
        else if(isumopt==1 || jsum==1) {
          for (i=0; i<lendip; i++) onetrace->data[i] = jsumdat[jsumn][i];
        } 
        else if(isumopt==2 || jsum==2) {
          if(ilinear==0 || fabs(outputloc-locit[jsumn])==fabs(outputloc-locit[jsumn2])) {
            for (i=0; i<lendip; i++) onetrace->data[i] = 0.5 * (jsumdat[jsumn][i] + jsumdat[jsumn2][i]);
          }
          else if(outputloc==locit[jsumn]) {
            for (i=0; i<lendip; i++) onetrace->data[i] = jsumdat[jsumn][i];
          }
          else if(outputloc==locit[jsumn2]) {
            for (i=0; i<lendip; i++) onetrace->data[i] = jsumdat[jsumn2][i];
          }
          else { /* note: both on 1 side is treated same as being on opposite */
            ratio = fabs(outputloc-locit[jsumn]) /* (both==0 checked at top) */ 
                 / (fabs(outputloc-locit[jsumn])+fabs(outputloc-locit[jsumn2]));
            for (i=0; i<lendip; i++) onetrace->data[i] = (1.-ratio) * jsumdat[jsumn][i] + ratio*jsumdat[jsumn2][i];
          }
        }
        else if(isumopt==3) { /* stack */
          for (i=0; i<lendip; i++) {
            onetrace->data[i] = 0.;
            for (j=0; j<jsum; j++) onetrace->data[i] += jsumdat[j][i];
            onetrace->data[i] /= jsum;
          }
        }
        else if(isumopt==4) { /* sum */
          for (i=0; i<lendip; i++) {
            onetrace->data[i] = 0.;
            for (j=0; j<jsum; j++) onetrace->data[i] += jsumdat[j][i];
          }
        }

        else { /* if (isumopt>4) then  dad or dadsum */
/*                                                                            */
/* So, what is going on here with this fault option?                          */
/* Consider a horizon of about amplitude 9 that ENDS.                         */
/* It looks kind of like this: 9 9 9 9 9 0 0 0 0 0 (assuming noise of 0).     */
/* The first loop below is computing relative location of average amplitude   */
/* in the summing window. Let us say the summing window length is 5.          */
/* So, as we roll along, we see windows of 9 9 9 9 9 which have an average    */
/* relative location of 0. When we get to the 9 9 9 9 0 it has a relative     */
/* average location of -0.5 which implies that we want to sum the values just */
/* of the first 4 locations. On the other hand, where a horizon STARTS it     */
/* looks like 0 0 0 0 9 and we ALSO want to sum just the first 4 locations.   */
/* (Fiddling around to get that symmetry is what is happening after wsmooth). */
/*                                                                            */
          centloc /= jsum;
          for (i=0; i<lendip; i++) {
            oneand[i] = 0.0;
            onetmp[i] = 1.e-30;
            for (j=0; j<jsum; j++) {         
              k = (j+klow) % jsum; /* known increasing order in rolling buffer*/
              oneand[i] += fabs(jsumdat[k][i]) * (float)(locit[k] - centloc);             
              onetmp[i] += fabs(jsumdat[k][i]);             
            }
            if(onetmp[i]/jsum > dadnoise) {
              oneand[i] = oneand[i] / onetmp[i];
            }
            else {
              oneand[i] = 0.0;
              onetmp[i] = 1.e-30;
            }
          }

/* Smooth the relative locations in time (weight by the sum of abs amplitude).*/
/* To call this smoothing is misleading since the weight actually causes the  */
/* relative location of faults in strong horizons to expand up and down in    */
/* time. The expansion is the primary desire, not the smoothing.              */

          wsmooth (oneand,onetmp,lendip,mindad,maxdad,onetm2);

          for (i=0; i<lendip; i++) {

/* Conveniently double and shift the smoothed relative locations.             */

            onetm2[i] =  2. * onetm2[i] * dadscale + (locmaxsum-locminsum);         

/* Reset some relative locations so 9 9 9 9 0 is treated same as 0 0 0 0 9    */

            if(onetm2[i]>locmaxsum-locminsum+0.1) onetm2[i] = onetm2[i] - (locmaxsum-locminsum) - 1.0;

/* So now sum-up either FROM the lower end, or TO the higher end.             */

            if(onetm2[i]>(locmaxsum-locminsum)/2. - 0.5) {
              jfirst = 0;
              jlast  = (int)(1.5+onetm2[i]);
              if(jlast>jsum) jlast=jsum;
            }
            else {
              jfirst = (int)(1.5+onetm2[i]);
              jlast  = jsum;
              if(jfirst<0) jfirst=0;
            }
            onetrace->data[i] = 0.0;
            for (j=jfirst; j<jlast; j++) {
              k = (j+klow) % jsum; 
              onetrace->data[i] += jsumdat[k][i];           
            }
            if(isumopt==5 && jlast-jfirst>1) onetrace->data[i] /= (jlast-jfirst);           
          }

        } /* end of  else if(isumopt>4) */

        if(iput>0) {
          puttr(onetrace); 
          mproct++;

          if(dipuse<1) {
            if(outdip>0) {
              for (i=0; i<lendip; i++) onetrace->data[i] = onedip[i] * dt; 
              fputtr(fpoutdip,onetrace); 
            }
            if(outpow>0) {
              for (i=0; i<lendip; i++) onetrace->data[i] = onepw2[i]; 
              fputtr(fpoutpow,onetrace); 
            }
            if(outsem>0) {
              for (i=0; i<lendip; i++) onetrace->data[i] = onesm2[i]; 
              fputtr(fpoutsem,onetrace); 
            }
          } 
        }  /* end of  if(iput>0)   */

      } /* end of   if(jsum>0) { */

      outputloc += outputinc; /* note use of the actual increment here */

    } /* end of  while(outputloc < maxstore */

    if(newgroup>0) { /* Reset location values to out-of-range, and so on.     */ 
      for (i=0; i<numrolling; i++) { 
        rollloc[i] = fromhead(tr,lockase) + 3.0*(locminscan + locminsum) - i - 5;
      }   
      outputloc = fromhead(tr,lockase) * outputinc + outputalign + locminscan + locminsum -1.0; 
      iroll = 0;
      iend = 0;
    }

/* After (trying to) scan the outputloc, use the rolling buffers to store    */
/* input traces and values of next location. Remember that the trace already */
/* read-into tr is the first trace to store for THIS input location.         */

    iroll++;
    if(iroll==numrolling) iroll = 0;
    rollloc[iroll] = fromhead(tr,lockase);
    rolldun[iroll] = 0;
    memcpy(rolltrace[iroll],&tr,sizeof(segy));
    maxstoredloc = fromhead(tr,lockase);

    if(indip>0 && dipuse>0 && icycle>0) { 
      if(!fgettr(fpindip,&trd)) 
        err("next input seismic trace needs a trace from indip file: %s   but get failed (near location=%d)",
          dname,rollloc[iroll]); 
      if((double)rollloc[iroll] != fromhead(trd,lockase)) 
        err("lockey value for dipfile trace %g   and input seismic trace %d are not the same ",
          fromhead(trd,lockase),rollloc[iroll]); 
      for (i=0; i<lendip; i++) rolldip[iroll][i] = trd.data[i];
    }

  } /* end of  while(icycle==1) {  */

  if(indip>0) efclose(fpindip);
  if(outdip>0) efclose(fpoutdip);
 
  warn("Number of traces input=%d.  Number output=%d.",nproct,mproct);
  if(locdups>0) {
    if(usegroup<1) warn("There were %d input traces with duplicate lockey values.",locdups);
    else warn("There were %d input traces with duplicate lockey values (within their groups).",locdups);
  }

  return 0;

}

/* Semblance Scanner                                                          */
/*                                                                            */
/* Semblance herein is computed as:                                           */
/*                                                                            */
/*  (Square of (Sum of amplitudes in locminscan to locmaxscan range)) /       */
/*             (Sum of squared amplitudes in that range))                     */
/*             divided by the number in that range.                           */
/*                                                                            */
/* Scans from smin to smin*sstep*(jsmax-1) are done for each sample time and  */
/* the maximum semblance value computed as above is found. That maximum is    */
/* then 3-point interpolated using the power at that maximum and the power    */
/* at its 2 neighbours (i.e. the values of their denominators above).         */
/* Note that this 3 point interpolation improves the result                   */
/* and also has the benefit of reducing the sstep granularity.                */
/*   Note this semblance is computed over just 1 time sample, but see dipext. */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* atrace     = The input traces.                                             */
/* aloc       = Location value for each input trace.                          */
/* numrolling = Number of traces in atrace and locations in aloc.             */
/* scanloc    = The location value where the scan takes place.                */
/* locminscan = The minimum relative location to include in scan (inclusive). */
/* locmaxscan = The maximum relative location to include in scan (inclusive). */
/* smin       = Minimum relative sample to scan.                              */
/* sstep      = Fractional sample interval to scan.                           */
/* jsmax      = Number of intervals to scan (from smin, increment by sstep).  */
/*                                                                            */
/* adip       = Output apparant dip (in samples per 1 location difference).   */
/* apow       = Output power along the chosen dip.                            */
/* asem       = Output semblance of the chosen dip.                           */
/* awrk       = Work buffer.                                                  */
/* awrq       = Work buffer.                                                  */
/* arng       = Work buffer (size: numrolling).                               */
/* ifori      = Work buffer (size: numrolling by jsmax)                       */
/* afori      = Work buffer (size: numrolling by jsmax)                       */
/*                                                                            */
/* lendip     = Length of traces and adip,apow,asem,awrk,awrq.                */
/* numout     = Number of traces in scanloc+locminscan to scanloc+locminscan. */
/*              0 returns all adip=0 and asem=0 and apow=0                    */
/*              1 returns all adip=0 and asem=1 and apow=trace amp squared    */


void scanit (segy **atrace, int *aloc, int numrolling, double scanloc, 
            double locminscan, double locmaxscan, float smin, float sstep, int jsmax,
            float *adip, float *apow, float *asem, float *awrk, float *awrq, 
            int *arng, int **ifori, float **afori, int lendip, int *numout) {

  int i=0;
  int j=0;
  int m=0;
  int n=0;
  float alin=0.;
  float aval=0.;
  float sval=0.;
  int numinrange=0;

  float thesem=0.;
  int thej=0; 
  float sl=0.;
  float sr=0.;
  float sp=0.;

  for (n=0; n<lendip; n++) {
    adip[n] = 0.;
    apow[n] = 0.;
    asem[n] = 0.;
  }

  for (i=0; i<numrolling; i++) {
    if(aloc[i] >= scanloc+locminscan && aloc[i] <= scanloc+locmaxscan) {
      arng[numinrange] = i;
      numinrange++;
    }
  }

  *numout = 0;
  if(numinrange<1) return; 
  *numout = 1;

/* If only 1 trace in range, then dip is presumed 0 (as initialized above)    */
/* And set power and semblance values correctly.                              */

  if(numinrange<2) {
    for (n=0; n<lendip; n++) {
      apow[n] = atrace[arng[0]]->data[n] * atrace[arng[0]]->data[n];
      asem[n] = 1.;
    }
    return; 
  }

/* Precompute the whole and fractional samples for linear interpolation so    */
/* it does not get repeated for every trace sample.                           */
/* (It is possible the compiler is smart enough to do this, but ????)         */
/* Note to future programmers: you could also use ints8r (or similar)         */
/*      to get more accurate seismic values than linear interpolation.        */
/*      But that would require passing a buffer to hold the shifted traces.   */

  for(j=0; j<jsmax; j++) {
    sval = smin + j*sstep;
    for (i=0; i<numinrange; i++) {
      m = ((int) (sval*(aloc[arng[i]]-scanloc)));
      if(sval*(aloc[arng[i]]-scanloc) > 0.) m++;
      alin = m - (sval*(aloc[arng[i]]-scanloc));
      ifori[i][j] = m;
      afori[i][j] = alin;
    }
  }

  for (n=0; n<lendip; n++) {                 
    for(j=0; j<jsmax; j++) {
      awrk[j] = 0.;                                                                
      awrq[j] = 1.e-30;  /* prevents divide by 0 when all samples = 0.0       */
      for (i=0; i<numinrange; i++) {
        m = ifori[i][j];
        if(n+m > 0 && n+m < lendip) { /* totals near top,bottom != numinrange */
          alin = afori[i][j];        
          aval = alin * atrace[arng[i]]->data[n+m-1] + (1.-alin) * atrace[arng[i]]->data[n+m];
          awrk[j] += aval;
          awrq[j] += aval*aval;
        }
      } 
    } /* end of  for(j=0; j<jsmax; j++) { */ 

/* Keep biggest semblance value and which scan number this is (thej).         */

    thesem = 0.;
    thej = jsmax/2; /* in case all zeroes (like a mute zone) */
    for(j=0; j<jsmax; j++) {
      if(awrk[j]*awrk[j]/awrq[j] > thesem) {
        thesem  = awrk[j]*awrk[j]/awrq[j];
        thej    = j;
      }
    } /* end of  for(j=0; j<jsmax; j++) { */

 /* At this point in the code, the dip values have sstep granularity.         */
 /* So perform 3 point interpolation around the maximum semblance value.      */
 /* This is done because switching from one sstep value to another might      */
 /* cause some effects if a reflection has slowly changing actual dip.        */
 /* Also, it allows users easier testing of what sstep is good enough (if the */
 /* 3 point interpolated dip of a big sstep gives as good results as the      */
 /* 3 point interpolated dip of a small sstep, use big and save CPU time).    */
           
    if(thesem < 1.e-30) {   /* if all zeroes (mute zones).                    */
      adip[n] = 0.0;  
      asem[n] = 0.0;  
      apow[n] = 0.0;
    }
    else if(thej>0 && thej<jsmax-1) {
      sl = awrk[thej-1]*awrk[thej-1]/awrq[thej-1];
      sr = awrk[thej+1]*awrk[thej+1]/awrq[thej+1];
      sp = 0.5 * (sl-sr) / (sl - 2.0*thesem + sr + 1.e-30);
      adip[n] = smin + thej*sstep + sp*sstep;     /* dip at interpolated peak */
      asem[n] = (thesem - 0.25 * (sl-sr) * sp)/numinrange; /* semblnc of peak */
      apow[n] = awrq[thej]/numinrange; /* power along dip of max semblnc pick */
    }
    else {
      adip[n] = smin + thej*sstep;
      asem[n] = thesem/numinrange;
      apow[n] = awrq[thej]/numinrange;
    }

  } /* end of  for (n=0; n<lendip; n++) { */ 

  return;
}

/* Weighted smoothing                                                         */
/*                                                                            */
/* val = input values                                                         */
/* wgt = input weights                                                        */
/* len = length of val,wgt,sval.                                              */
/* minsd = lower range to include in sum for i-th value (usually = -maxsd)    */
/* maxsd = higher range to include in sum for i-th value (usually positive)   */
/*                                                                            */
/* sval = output smoothed values                                              */
/*                                                                            */
/* Notes:                                                                     */
/* 1. Weights must be >= 0 or you could get divide by 0. (not checked herein) */
/* 2. Could do this by add and subtract to running totals as i increments,    */
/*    which would be faster for big minsd,maxsd. But do not expect that here. */
/*    (And the compiler-optimizer will unroll the inner loop).                */

void wsmooth (float *val, float *wgt, int len, int minsd, int maxsd, float *sval) {

  int i=0;
  int k=0;
  float twgt=0.;
  float tval=0.;

  for(i=0-minsd; i<len-maxsd; i++) { 
    twgt = 1.e-30;
    tval = 0.;
    for(k=minsd; k<=maxsd; k++) {
      twgt += wgt[i+k];
      tval += wgt[i+k] * val[i+k];
    }
    sval[i] = tval / twgt;
  }

  for(i=0; i<0-minsd; i++) sval[i] = sval[0-minsd]; 
  for(i=len-maxsd; i<len; i++) sval[i] = sval[len-maxsd-1];

  return;
}
