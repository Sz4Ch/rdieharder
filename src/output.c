/*
 *========================================================================
 * See copyright in copyright.h and the accompanying file COPYING
 *========================================================================
 */

#include "dieharder.h"

//void table_line(Dtest *dtest,Test **test);
void table_line();

void output(Dtest *dtest,Test **test)
{

/*
 * This is a one-stop-shop output routine for standard output in all
 * run_whatever_test() routines.  It manages all output format choices,
 * specifically the choice between report vs table output and the
 * control of just what fields or options are selected for output.
 */

 /*
  * This almost certainly belongs in the show_test_results section,
  * possibly with additional conditionals rejecting test results involving
  * rewinds, period.
  */
 if(strncmp("file_input",gsl_rng_name(rng),10) == 0){
   if(!quiet && !table){
       Rprintf("# %u rands were used in this test\n",(unsigned int)file_input_get_rtot(rng));
   }
   if(file_input_get_rewind_cnt(rng) != 0){
     REprintf("# The file %s was rewound %u times\n",gsl_rng_name(rng),file_input_get_rewind_cnt(rng));
   }
 }

 /*
  * Show standard test results, for all statistics generated by a single run.
  * Output can either be a report (with histogram and all) or a table
  * line.
  */
 if(table) {
     //table_line(dtest,test);
     table_line();
 } else {
   if(!quiet){
     show_test_header(dtest,test);
   }
   show_test_results(dtest,test);
 }

}

/*
 * Write a standard test header.
 */
void show_test_header(Dtest *dtest,Test **test)
{

 /*
  * Show standard test description
  */
 Rprintf("%s",dtest->description);

 /*
  * This is the non-custom part of the header, which should be enough
  * for nearly all tests by design.
  */
 Rprintf("#                        Run Details\n");

 /*
  * Show the generator being tested, handling file_input generators
  * separately.
  */
 if(strncmp("file_input",gsl_rng_name(rng),10) == 0){
   Rprintf("# Random number generator tested: %s\n",gsl_rng_name(rng));
   Rprintf("# File %s contains %u rands of %c type.\n",filename,(unsigned int)filecount,filetype);
 } else {
   Rprintf("# Random number generator tested: %s\n",gsl_rng_name(rng));
 }

 /*
  * Now show both current and default values for tsamples and psamples.
  */
 Rprintf("# Samples per test pvalue = %u (test default is %u)\n",test[0]->tsamples,dtest->tsamples_std);
 Rprintf("# P-values in final KS test = %u (test default is %u)\n",test[0]->psamples,dtest->psamples_std);

}


/*
 * Write a standard test footer.  Arguments include the test
 * variables, the test pvalue, the ks_pvalues, and a test description.
 */
void show_test_results(Dtest *dtest,Test **test)
{

 unsigned int i;

 /*
  * There may be more than one statistic (final p-value) generated by
  * this test; we loop over all of them.
  */
 /* printf("In show_test_results for %s for %u statistics\n",dtest->name,dtest->nkps); */
 for(i=0;i<dtest->nkps;i++){

   /*
    * Display histogram of ks p-values (optional)
    */
   if(hist_flag && quiet == 0){
     /* Debugging
     for(j=0;j<test[i]->psamples;j++){
       printf("pvalue = %f\n",test[i]->pvalues[j]);
       fflush(stdout);
     }
     Debugging */
     histogram(test[i]->pvalues,test[i]->pvlabel,test[i]->psamples,0.0,1.0,10,"p-values");
   }

   /*
    * This isn't quiet-suppressible, as it is important information; we do
    * print it to stderr, though, so that one can fairly easily filter even
    * it away...
    */
   if(strncmp("file_input",gsl_rng_name(rng),10) == 0){
       REprintf("# %u rands were used in this test\n",(unsigned int)file_input_get_rtot(rng));
     REprintf("# The file %s was rewound %u times\n",gsl_rng_name(rng),file_input_get_rewind_cnt(rng));
   }

   #if !defined(RDIEHARDER)
   if(quiet == 0){
     Rprintf("#                          Results\n");
   }
   if(test[i]->psamples == 1){
     printf("Single test: p = %10.8f\n",test[i]->ks_pvalue);
     printf("Assessment: ");
     /* Work through some ranges here */
     if(test[i]->ks_pvalue < 0.0001 || test[i]->ks_pvalue > 0.9999){
       printf("FAILED at < 0.02%% for %s\n",dtest->name);
     } else if(test[i]->ks_pvalue < 0.01 || test[i] -> ks_pvalue > 0.99){
       printf("POOR at < 2%% for %s\n",dtest->name);
       printf("Recommendation:  Repeat test to verify failure.\n");
     } else {
       printf("PASSED at > 2%% for %s\n",dtest->name);
     }
   } else {
     printf("Kuiper KS: p = %10.8f\n",test[i]->ks_pvalue);
     printf("Assessment: ");
     /* Work through some ranges here */
     if(test[i]->ks_pvalue < 0.0001){
       printf("FAILED at < 0.01%% for %s\n",dtest->name);
     } else if(test[i]->ks_pvalue < 0.01){
       printf("POOR at < 1%% for %s\n",dtest->name);
       printf("Recommendation:  Repeat test to verify failure.\n");
     } else if(test[i]->ks_pvalue < 0.05){
       printf("POSSIBLY WEAK at < 5%% for %s\n",dtest->name);
       printf("Recommendation:  Repeat test to verify failure.\n");
     } else {
       printf("PASSED at > 5%% for %s\n",dtest->name);
     }
   }
   #endif  /* !defined(RDIEHARDER) */
 }

 #if defined(RDIEHARDER)
 save_values_for_R(dtest, test);
 #endif /* RDIEHARDER */
}

/*
 * Print out all results in table format only
 */
//void table_line(Dtest *dtest,Test **test)
void table_line()
{

#if !defined(RDIEHARDER)
 unsigned int i,j,k;

 /*
  * There may be more than one statistic (final p-value) generated by
  * this test; we loop over all of them.
  */
 for(i=0;i<dtest->nkps;i++){

   if(tflag & TNAME){
     Rprintf("|");
     if(fromfile){
       Rprintf("%-30s",filename);
     } else {
       Rprintf("%-30s",dtest->name);
     }
   }
   if(tflag & TSNAME){
     Rprintf("|");
     if(fromfile){
       Rprintf("%-20.20s",filename);
     } else {
       Rprintf("%-20.20s",dtest->sname);
     }
   }
   if(tflag & TNTUPLE){
     Rprintf("|");
     Rprintf("%6d",test[i]->ntuple);
   }
   if(tflag & TTSAMPLES){
     Rprintf("|");
     Rprintf("%12u",test[0]->tsamples);
   }
   if(tflag & TPSAMPLES){
     Rprintf("|");
     Rprintf("%8u",test[0]->psamples);
   }
   if(tflag & TPVALUE){
     Rprintf("|");
     Rprintf("%10.8f",test[i]->ks_pvalue);
   }
   if(tflag & TASSESSMENT){
     Rprintf("|");
     if(test[i]->ks_pvalue < 0.0001 || test[i]->ks_pvalue > 0.9999){
       Rprintf("%16s","FAILED(0.02%) ");
     } else if(test[i]->ks_pvalue < 0.01 || test[i]->ks_pvalue > 0.99){
       Rprintf("%16s","MAY BE WEAK(2%)");
     } else {
       Rprintf("%16s","PASSED     ");
     }
   }

   Rprintf("|\n");

 }

#endif  /* !defined(RDIEHARDER) */

}

void table_header()
{

 #if !defined(RDIEHARDER)
 unsigned int i,j,k;

 /*
  * We assemble the table header according to what tflag's value is.
  */
 if(tflag & THEADER == 0) return;   /* No header! */

 Rprintf("#==============================================================================\n");
 if(tflag & TGEN){
   Rprintf("#| Testing generator %s",gsl_rng_name(rng));
   if(fromfile){
     Rprintf(" (from file %s)",filename);
   } else {
     Rprintf(" -- Rands per second = %e.\n",rng_rands_per_second);
   }
 }
 
 Rprintf("#");
 if(tflag & TNAME){
   Rprintf("%30s","Test (short) Name");
 }
 if(tflag & TSNAME){
   Rprintf("%20s","Test (short) Name  ");
 }
 if(tflag & TNTUPLE){
   Rprintf("|");
   Rprintf("%6s","ntuple");
 }
 if(tflag & TTSAMPLES){
   Rprintf("|");
   Rprintf("%12s","  tsamples  ");
 }
 if(tflag & TPSAMPLES){
   Rprintf("|");
   Rprintf("%8s","psamples");
 }
 if(tflag & TPVALUE){
   Rprintf("|");
   Rprintf("%10s","p-value ");
 }
 if(tflag & TASSESSMENT){
   Rprintf("|");
   Rprintf("%16s","Assessment   ");
 }
 Rprintf("|\n");
 Rprintf("#==============================================================================\n");

#endif  /* !defined(RDIEHARDER) */

}

