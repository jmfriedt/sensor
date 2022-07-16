#include "DDSinc.h"

#define NB_SLIDE 10

int slid[NB_SLIDE*ANTENNES_MAX*NBPICS_MAX/2];
int somme_slid[ANTENNES_MAX*NBPICS_MAX/2],index_slid[ANTENNES_MAX*NBPICS_MAX/2],filled_slid[ANTENNES_MAX*NBPICS_MAX/2];

void init_sliding_avg()
{int k;
 for (k=0;k<ANTENNES_MAX*NBPICS_MAX/2;k++)  // les moyennes sont indexees sur chaque antenne/
    {somme_slid[k]=0;                       //    chaque paire de resonance
     index_slid[k]=0;
     filled_slid[k]=0;
    }
}

int sliding_avg(int val,int antenne,int npic)
{somme_slid[antenne*NBPICS_MAX/2+npic]+=val;
 if (filled_slid[antenne*NBPICS_MAX/2+npic]==1) 
    somme_slid[antenne*NBPICS_MAX/2+npic]-=slid[index_slid[antenne*NBPICS_MAX/2+npic]+(antenne*NBPICS_MAX/2+npic)*NB_SLIDE];
 slid[index_slid[antenne*NBPICS_MAX/2+npic]+(antenne*NBPICS_MAX/2+npic)*NB_SLIDE]=val;
 index_slid[antenne*NBPICS_MAX/2+npic]++;
 if (index_slid[antenne*NBPICS_MAX/2+npic]==NB_SLIDE)
    {index_slid[antenne*NBPICS_MAX/2+npic]=0;
     filled_slid[antenne*NBPICS_MAX/2+npic]=1;
    }
 if (filled_slid[antenne*NBPICS_MAX/2+npic]==1)
    return(somme_slid[antenne*NBPICS_MAX/2+npic]/NB_SLIDE);
 else 
    {if ((index_slid[antenne*NBPICS_MAX/2+npic]==1)&&(filled_slid[antenne*NBPICS_MAX/2+npic]==0))
        return(somme_slid[antenne*NBPICS_MAX/2+npic]);
     else return(somme_slid[antenne*NBPICS_MAX/2+npic]/(index_slid[antenne*NBPICS_MAX/2+npic]));
    }
}

