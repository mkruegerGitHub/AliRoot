/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/*
$Log$
*/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Time Projection Chamber                                                  //
//  This class contains the basic functions for the Time Projection Chamber  //
//  detector. Functions specific to one particular geometry are              //
//  contained in the derived classes                                         //
//                                                                           //
//Begin_Html
/*
<img src="picts/AliTPCClass.gif">
*/
//End_Html
//                                                                           //
//                                                                          //
///////////////////////////////////////////////////////////////////////////////

#include <TMath.h>
#include <TRandom.h>
#include <TVector.h>
#include <TMatrix.h>
#include <TGeometry.h>
#include <TNode.h>
#include <TTUBS.h>
#include <TObjectTable.h>
#include "TParticle.h"
#include "AliTPC.h"
#include "AliRun.h"
#include <iostream.h>
#include <fstream.h>
#include "AliMC.h"

//MI change
#include "AliTPCParam.h"
#include "AliTPCD.h"
#include "AliTPCPRF2D.h"
#include "AliTPCRF1D.h"



ClassImp(AliTPC) 

//_____________________________________________________________________________
AliTPC::AliTPC()
{
  //
  // Default constructor
  //
  fIshunt   = 0;
  fClusters = 0;
  fHits     = 0;
  fDigits   = 0;
  fTracks   = 0;
  fNsectors = 0;
  fNtracks  = 0;
  fNclusters= 0;

  fDigParam= new AliTPCD();
  fDigits = fDigParam->GetArray();
}
 
//_____________________________________________________________________________
AliTPC::AliTPC(const char *name, const char *title)
      : AliDetector(name,title)
{
  //
  // Standard constructor
  //

  //
  // Initialise arrays of hits and digits 
  fHits     = new TClonesArray("AliTPChit",  176);
  //  fDigits   = new TClonesArray("AliTPCdigit",10000);
  //MI change
  fDigParam= new AliTPCD;
  fDigits = fDigParam->GetArray();

  AliTPCParam  *fTPCParam = &(fDigParam->GetParam());

  //
  // Initialise counters
  //
  fClusters = 0;
  fTracks   = 0;
  fNsectors = fTPCParam->GetNSector();
  fNtracks  = 0;
  fNclusters= 0;
  fDigitsIndex = new Int_t[fNsectors+1];
  fClustersIndex = new Int_t[fNsectors+1];
  //
  fIshunt     =  0;
  //
  // Initialise color attributes
  SetMarkerColor(kYellow);
}

//_____________________________________________________________________________
AliTPC::~AliTPC()
{
  //
  // TPC destructor
  //
  fIshunt   = 0;
  delete fHits;
  delete fDigits;
  delete fClusters;
  delete fTracks;
  delete fDigParam;
  if (fDigitsIndex)   delete [] fDigitsIndex;
  if (fClustersIndex) delete [] fClustersIndex;
}

//_____________________________________________________________________________
void AliTPC::AddCluster(Float_t *hits, Int_t *tracks)
{
  //
  // Add a simulated cluster to the list
  //
  if(!fClusters) fClusters=new TClonesArray("AliTPCcluster",10000);
  TClonesArray &lclusters = *fClusters;
  new(lclusters[fNclusters++]) AliTPCcluster(hits,tracks);
}
 
//_____________________________________________________________________________
void AliTPC::AddCluster(const AliTPCcluster &c)
{
  //
  // Add a simulated cluster copy to the list
  //
  if(!fClusters) fClusters=new TClonesArray("AliTPCcluster",10000);
  TClonesArray &lclusters = *fClusters;
  new(lclusters[fNclusters++]) AliTPCcluster(c);
}
 
//_____________________________________________________________________________
void AliTPC::AddDigit(Int_t *tracks, Int_t *digits)
{
  //
  // Add a TPC digit to the list
  //
  //  TClonesArray &ldigits = *fDigits;
  //MI change 
  TClonesArray &ldigits = *fDigParam->GetArray();
  new(ldigits[fNdigits++]) AliTPCdigit(tracks,digits);
}
 
//_____________________________________________________________________________
void AliTPC::AddHit(Int_t track, Int_t *vol, Float_t *hits)
{
  //
  // Add a hit to the list
  //
  TClonesArray &lhits = *fHits;
  new(lhits[fNhits++]) AliTPChit(fIshunt,track,vol,hits);
}
 
//_____________________________________________________________________________
void AliTPC::AddTrack(Float_t *hits)
{
  //
  // Add a track to the list of tracks
  //
  TClonesArray &ltracks = *fTracks;
  new(ltracks[fNtracks++]) AliTPCtrack(hits);
}

//_____________________________________________________________________________
void AliTPC::AddTrack(const AliTPCtrack& t)
{
  //
  // Add a track copy to the list of tracks
  //
  if(!fTracks) fTracks=new TClonesArray("AliTPCtrack",10000);
  TClonesArray &ltracks = *fTracks;
  new(ltracks[fNtracks++]) AliTPCtrack(t);
}

//_____________________________________________________________________________
void AliTPC::BuildGeometry()
{
  //
  // Build TPC ROOT TNode geometry for the event display
  //
  TNode *Node, *Top;
  TTUBS *tubs;
  Int_t i;
  const int kColorTPC=19;
  char name[5], title[25];
  const Double_t kDegrad=TMath::Pi()/180;
  const Double_t kRaddeg=180./TMath::Pi();

  AliTPCParam * fTPCParam = &(fDigParam->GetParam());

  Float_t InnerOpenAngle = fTPCParam->GetInnerAngle();
  Float_t OuterOpenAngle = fTPCParam->GetOuterAngle();

  Float_t InnerAngleShift = fTPCParam->GetInnerAngleShift();
  Float_t OuterAngleShift = fTPCParam->GetOuterAngleShift();

  Int_t nLo = fTPCParam->GetNInnerSector()/2;
  Int_t nHi = fTPCParam->GetNOuterSector()/2;  

  const Double_t loAng = (Double_t)TMath::Nint(InnerOpenAngle*kRaddeg);
  const Double_t hiAng = (Double_t)TMath::Nint(OuterOpenAngle*kRaddeg);
  const Double_t loAngSh = (Double_t)TMath::Nint(InnerAngleShift*kRaddeg);
  const Double_t hiAngSh = (Double_t)TMath::Nint(OuterAngleShift*kRaddeg);  


  const Double_t loCorr = 1/TMath::Cos(0.5*loAng*kDegrad);
  const Double_t hiCorr = 1/TMath::Cos(0.5*hiAng*kDegrad);

  Double_t rl,ru;
  

  //
  // Get ALICE top node
  //

  Top=gAlice->GetGeometry()->GetNode("alice");

  //  inner sectors

  rl = fTPCParam->GetInSecLowEdge();
  ru = fTPCParam->GetInSecUpEdge();
 

  for(i=0;i<nLo;i++) {
    sprintf(name,"LS%2.2d",i);
    name[4]='\0';
    sprintf(title,"TPC low sector %3d",i);
    title[24]='\0';
    
    tubs = new TTUBS(name,title,"void",rl*loCorr,ru*loCorr,250.,
                     loAng*(i-0.5)+loAngSh,loAng*(i+0.5)+loAngSh);
    tubs->SetNumberOfDivisions(1);
    Top->cd();
    Node = new TNode(name,title,name,0,0,0,"");
    Node->SetLineColor(kColorTPC);
    fNodes->Add(Node);
  }

  // Outer sectors

  rl = fTPCParam->GetOuSecLowEdge();
  ru = fTPCParam->GetOuSecUpEdge();

  for(i=0;i<nHi;i++) {
    sprintf(name,"US%2.2d",i);
    name[4]='\0';
    sprintf(title,"TPC upper sector %d",i);
    title[24]='\0';
    tubs = new TTUBS(name,title,"void",rl*hiCorr,ru*hiCorr,250,
                     hiAng*(i-0.5)+hiAngSh,hiAng*(i+0.5)+hiAngSh);
    tubs->SetNumberOfDivisions(1);
    Top->cd();
    Node = new TNode(name,title,name,0,0,0,"");
    Node->SetLineColor(kColorTPC);
    fNodes->Add(Node);
  }
}  
  
  

//_____________________________________________________________________________
Int_t AliTPC::DistancetoPrimitive(Int_t , Int_t )
{
  //
  // Calculate distance from TPC to mouse on the display
  // Dummy procedure
  //
  return 9999;
}

//_____________________________________________________________________________
static Double_t SigmaY2(Double_t r, Double_t tgl, Double_t pt)
{
  //
  // Parametrised error of the cluster reconstruction (pad direction)   
  //
  pt=TMath::Abs(pt)*1000.;
  Double_t x=r/pt;
  tgl=TMath::Abs(tgl);
  Double_t s=a_rphi - b_rphi*r*tgl + c_rphi*x*x + d_rphi*x;
  if (s<0.4e-3) s=0.4e-3;
  s*=1.3; //Iouri Belikov
  return s;
}

//_____________________________________________________________________________
static Double_t SigmaZ2(Double_t r, Double_t tgl) 
{
  //
  // Parametrised error of the cluster reconstruction (drift direction)
  //
  tgl=TMath::Abs(tgl);
  Double_t s=a_z - b_z*r*tgl + c_z*tgl*tgl;
  if (s<0.4e-3) s=0.4e-3;
  s*=1.3; //Iouri Belikov
  return s;
}

//_____________________________________________________________________________
inline Double_t f1(Double_t x1,Double_t y1,
                   Double_t x2,Double_t y2,
                   Double_t x3,Double_t y3) 
{
  //-----------------------------------------------------------------
  // Initial approximation of the track curvature
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  Double_t d=(x2-x1)*(y3-y2)-(x3-x2)*(y2-y1);
  Double_t a=0.5*((y3-y2)*(y2*y2-y1*y1+x2*x2-x1*x1)-
                  (y2-y1)*(y3*y3-y2*y2+x3*x3-x2*x2));
  Double_t b=0.5*((x2-x1)*(y3*y3-y2*y2+x3*x3-x2*x2)-
                  (x3-x2)*(y2*y2-y1*y1+x2*x2-x1*x1));

  Double_t xr=TMath::Abs(d/(d*x1-a)), yr=d/(d*y1-b);

  return -xr*yr/sqrt(xr*xr+yr*yr); 
}


//_____________________________________________________________________________
inline Double_t f2(Double_t x1,Double_t y1,
                   Double_t x2,Double_t y2,
                   Double_t x3,Double_t y3) 
{
  //-----------------------------------------------------------------
  // Initial approximation of the track curvature times center of curvature
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  Double_t d=(x2-x1)*(y3-y2)-(x3-x2)*(y2-y1);
  Double_t a=0.5*((y3-y2)*(y2*y2-y1*y1+x2*x2-x1*x1)-
                  (y2-y1)*(y3*y3-y2*y2+x3*x3-x2*x2));
  Double_t b=0.5*((x2-x1)*(y3*y3-y2*y2+x3*x3-x2*x2)-
                  (x3-x2)*(y2*y2-y1*y1+x2*x2-x1*x1));

  Double_t xr=TMath::Abs(d/(d*x1-a)), yr=d/(d*y1-b);
  
  return -a/(d*y1-b)*xr/sqrt(xr*xr+yr*yr);
}

//_____________________________________________________________________________
inline Double_t f3(Double_t x1,Double_t y1, 
                   Double_t x2,Double_t y2,
                   Double_t z1,Double_t z2) 
{
  //-----------------------------------------------------------------
  // Initial approximation of the tangent of the track dip angle
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  return (z1 - z2)/sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

//_____________________________________________________________________________
static int FindProlongation(AliTPCtrack& t, const AliTPCSector *sec,
			    int s, int rf=0) 
{
  //-----------------------------------------------------------------
  // This function tries to find a track prolongation.
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  const int ROWS_TO_SKIP=int(0.5*sec->GetNRows());
  const Float_t MAX_CHI2=12.;
  int try_again=ROWS_TO_SKIP;
  Double_t alpha=sec->GetAlpha();
  int ns=int(2*TMath::Pi()/alpha+0.5);

  for (int nr=sec->GetRowNumber(t.GetX())-1; nr>=rf; nr--) {
    Double_t x=sec->GetX(nr), ymax=sec->GetMaxY(nr);
    if (!t.PropagateTo(x)) return 0;

    AliTPCcluster *cl=0;
    Double_t max_chi2=MAX_CHI2;
    const AliTPCRow& row=sec[s][nr];
    Double_t sy2=SigmaY2(t.GetX(),t.GetTgl(),t.GetPt());
    Double_t sz2=SigmaZ2(t.GetX(),t.GetTgl());
    Double_t road=5.*sqrt(t.GetSigmaY2() + sy2), y=t.GetY(), z=t.GetZ();

    if (road>30) {
      if (t>4) cerr<<t<<" FindProlongation warning: Too broad road !\n"; 
      return 0;
    }

    if (row) {
      for (int i=row.Find(y-road); i<row; i++) {
	AliTPCcluster* c=(AliTPCcluster*)(row[i]);
	if (c->fY > y+road) break;
	if (c->IsUsed()) continue;
	if ((c->fZ - z)*(c->fZ - z) > 25.*(t.GetSigmaZ2() + sz2)) continue;
	Double_t chi2=t.GetPredictedChi2(c);
	if (chi2 > max_chi2) continue;
	max_chi2=chi2;
	cl=c;       
      }
    }
    if (cl) {
      t.Update(cl,max_chi2);
      Double_t ll=TMath::Sqrt((1+t.GetTgl()*t.GetTgl())/
               (1-(t.GetC()*x-t.GetEta())*(t.GetC()*x-t.GetEta())));
      cl->fdEdX = cl->fQ/ll;
      try_again=ROWS_TO_SKIP;
    } else {
      if (try_again==0) break;
      if (y > ymax) {
         s = (s+1) % ns;
         if (!t.Rotate(alpha)) return 0;
      } else if (y <-ymax) {
         s = (s-1+ns) % ns;
         if (!t.Rotate(-alpha)) return 0;
      }
      try_again--;
    }
  }

  return 1;
}


//_____________________________________________________________________________
static void MakeSeeds(TObjArray& seeds,const AliTPCSector *sec, int max_sec,
int i1, int i2)
{
  //-----------------------------------------------------------------
  // This function creates track seeds.
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  TMatrix C(5,5); TVector x(5);
  double alpha=sec->GetAlpha(), shift=sec->GetAlphaShift();
  double cs=cos(alpha), sn=sin(alpha);
  for (int ns=0; ns<max_sec; ns++) {
    int nl=sec[(ns-1+max_sec)%max_sec][i2];
    int nm=sec[ns][i2];
    int nu=sec[(ns+1)%max_sec][i2];
    const AliTPCRow& r1=sec[ns][i1];
    for (int is=0; is < r1; is++) {
      double x1=sec->GetX(i1), y1=r1[is]->fY, z1=r1[is]->fZ;
      for (int js=0; js < nl+nm+nu; js++) {
	const AliTPCcluster *cl;
	int ks;
        double x2=sec->GetX(i2), y2, z2, tmp;

	if (js<nl) {
	  ks=(ns-1+max_sec)%max_sec;
	  const AliTPCRow& r2=sec[(ns-1+max_sec)%max_sec][i2];
	  cl=r2[js];
          y2=cl->fY; z2=cl->fZ;
          tmp= x2*cs+y2*sn;
          y2 =-x2*sn+y2*cs; x2=tmp;
	} else 
	  if (js<nl+nm) {
	    ks=ns;
	    const AliTPCRow& r2=sec[ns][i2];
	    cl=r2[js-nl];
            y2=cl->fY; z2=cl->fZ;
	  } else {
	    ks=(ns+1)%max_sec;
	    const AliTPCRow& r2=sec[(ns+1)%max_sec][i2];
	    cl=r2[js-nl-nm];
            y2=cl->fY; z2=cl->fZ;
            tmp=x2*cs-y2*sn;
            y2 =x2*sn+y2*cs; x2=tmp;
	  }

        double d=(x2-x1)*(0.-y2)-(0.-x2)*(y2-y1);
        if (d==0.) {cerr<<"MakeSeeds warning: Straight seed !\n"; continue;}

	x(0)=y1;
	x(1)=z1;
	x(2)=f1(x1,y1,x2,y2,0.,0.);
	x(3)=f2(x1,y1,x2,y2,0.,0.);
	x(4)=f3(x1,y1,x2,y2,z1,z2);
	
	if (TMath::Abs(x(2)*x1-x(3)) >= 0.999) continue;
	
	if (TMath::Abs(x(4)) > 1.2) continue;

	Double_t a=asin(x(3));
	Double_t zv=z1 - x(4)/x(2)*(a+asin(x(2)*x1-x(3)));
	if (TMath::Abs(zv)>33.) continue; 

	TMatrix X(6,6); X=0.; 
	X(0,0)=r1[is]->fSigmaY2; X(1,1)=r1[is]->fSigmaZ2;
	X(2,2)=cl->fSigmaY2;     X(3,3)=cl->fSigmaZ2;
	X(4,4)=3./12.; X(5,5)=3./12.;
	TMatrix F(5,6); F.UnitMatrix();
	Double_t sy=sqrt(X(0,0)), sz=sqrt(X(1,1));
	F(2,0)=(f1(x1,y1+sy,x2,y2,0.,0.)-x(2))/sy;
	F(2,2)=(f1(x1,y1,x2,y2+sy,0.,0.)-x(2))/sy;
	F(2,4)=(f1(x1,y1,x2,y2,0.,0.+sy)-x(2))/sy;
	F(3,0)=(f2(x1,y1+sy,x2,y2,0.,0.)-x(3))/sy;
	F(3,2)=(f2(x1,y1,x2,y2+sy,0.,0.)-x(3))/sy;
	F(3,4)=(f2(x1,y1,x2,y2,0.,0.+sy)-x(3))/sy;
	F(4,0)=(f3(x1,y1+sy,x2,y2,z1,z2)-x(4))/sy;
	F(4,1)=(f3(x1,y1,x2,y2,z1+sz,z2)-x(4))/sz;
	F(4,2)=(f3(x1,y1,x2,y2+sy,z1,z2)-x(4))/sy;
	F(4,3)=(f3(x1,y1,x2,y2,z1,z2+sz)-x(4))/sz;
	F(4,4)=0;
	F(3,3)=0;
	
	TMatrix t(F,TMatrix::kMult,X);
	C.Mult(t,TMatrix(TMatrix::kTransposed,F));

	AliTPCtrack *track=new AliTPCtrack(r1[is], x, C, x1, ns*alpha+shift);
	int rc=FindProlongation(*track,sec,ns,i2);
        if (rc<0 || *track<(i1-i2)/2) delete track;
        else seeds.AddLast(track); 
      }
    }
  }
}

//_____________________________________________________________________________
AliTPCParam *AliTPCSector::param;
void AliTPC::Clusters2Tracks()
{
  //-----------------------------------------------------------------
  // This is a track finder.
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  if (!fClusters) return;

  AliTPCParam *p=&fDigParam->GetParam();
  AliTPCSector::SetParam(p);

  const int nis=p->GetNInnerSector()/2;
  AliTPCSSector *ssec=new AliTPCSSector[nis];         
  int nrow_low=ssec->GetNRows();     

  const int nos=p->GetNOuterSector()/2;
  AliTPCLSector *lsec=new AliTPCLSector[nos];
  int nrow_up=lsec->GetNRows();

  int ncl=fClusters->GetEntriesFast();
  while (ncl--) {
    AliTPCcluster *c=(AliTPCcluster*)fClusters->UncheckedAt(ncl);
    Int_t sec=c->fSector, row=c->fPadRow;
    if (sec<nis*2) {
      ssec[sec%nis][row].InsertCluster(c);
    } else {
      sec -= nis*2;
      lsec[sec%nos][row].InsertCluster(c);
    }
  }
  
  TObjArray seeds(20000);

  int nrows=nrow_low+nrow_up;
  int gap=int(0.125*nrows), shift=int(0.5*gap);
  MakeSeeds(seeds, lsec, nos, nrow_up-1, nrow_up-1-gap);
  MakeSeeds(seeds, lsec, nos, nrow_up-1-shift, nrow_up-1-shift-gap);
    
  seeds.Sort();
  
  int found=0;
  int nseed=seeds.GetEntriesFast();
  
  for (int s=0; s<nseed; s++) {
    AliTPCtrack& t=*((AliTPCtrack*)seeds.UncheckedAt(s));
    double alpha=t.GetAlpha();
    if (alpha > 2.*TMath::Pi()) alpha -= 2.*TMath::Pi();  
    if (alpha < 0.            ) alpha += 2.*TMath::Pi();  
    int ns=int(alpha/lsec->GetAlpha())%nos;

    if (!FindProlongation(t,lsec,ns)) continue;

    alpha=t.GetAlpha() + 0.5*ssec->GetAlpha() - ssec->GetAlphaShift();
    if (alpha > 2.*TMath::Pi()) alpha -= 2.*TMath::Pi();
    if (alpha < 0.            ) alpha += 2.*TMath::Pi();
    ns=int(alpha/ssec->GetAlpha())%nis; //index of the inner sector needed

    alpha=ns*ssec->GetAlpha() - t.GetAlpha();
    if (!t.Rotate(alpha)) continue;

    if (!FindProlongation(t,ssec,ns)) continue;
    
    if (t < int(0.4*nrows)) continue;
    
    AddTrack(t);
    t.UseClusters();
    cerr<<found++<<'\r';
  }  

  delete[] ssec;
  delete[] lsec;
}

//_____________________________________________________________________________
void AliTPC::CreateMaterials()
{
  //-----------------------------------------------
  // Create Materials for for TPC
  //-----------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  Int_t ISXFLD=gAlice->Field()->Integ();
  Float_t SXMGMX=gAlice->Field()->Max();

  Float_t amat[5]; // atomic numbers
  Float_t zmat[5]; // z
  Float_t wmat[5]; // proportions

  Float_t density;

  //  ********************* Gases *******************

  //--------------------------------------------------------------
  // pure gases
  //--------------------------------------------------------------

  // Ne


  Float_t a_ne = 20.18;
  Float_t z_ne = 10.;
  
  density = 0.0009;

  AliMaterial(20,"Ne",a_ne,z_ne,density,999.,999.);

  // Ar

  Float_t a_ar = 39.948;
  Float_t z_ar = 18.;

  density = 0.001782;
 
  AliMaterial(21,"Ar",a_ar,z_ar,density,999.,999.);

  Float_t a_pure[2];
  
  a_pure[0] = a_ne;
  a_pure[1] = a_ar;
  

  //--------------------------------------------------------------
  // gases - compounds
  //--------------------------------------------------------------

  Float_t amol[3];

  //  CO2

  amat[0]=12.011;
  amat[1]=15.9994;

  zmat[0]=6.;
  zmat[1]=8.;

  wmat[0]=1.;
  wmat[1]=2.;

  density=0.001977;

  amol[0] = amat[0]*wmat[0]+amat[1]*wmat[1];

  AliMixture(10,"CO2",amat,zmat,density,-2,wmat);

  // CF4

  amat[0]=12.011;
  amat[1]=18.998;

  zmat[0]=6.;
  zmat[1]=9.;
 
  wmat[0]=1.;
  wmat[1]=4.;
 
  density=0.003034;

  amol[1] = amat[0]*wmat[0]+amat[1]*wmat[1];

  AliMixture(11,"CF4",amat,zmat,density,-2,wmat); 

  // CH4

  amat[0]=12.011;
  amat[1]=1.;

  zmat[0]=6.;
  zmat[1]=1.;

  wmat[0]=1.;
  wmat[1]=4.;

  density=0.000717;

  amol[2] = amat[0]*wmat[0]+amat[1]*wmat[1];

  AliMixture(12,"CH4",amat,zmat,density,-2,wmat);

  //----------------------------------------------------------------
  // gases - mixtures, ID >= 20 pure gases, <= 10 ID < 20 -compounds
  //----------------------------------------------------------------
 
  char namate[21];
 
  density = 0.;
  Float_t am=0;
  Int_t nc;

  Float_t a,z,rho,absl,X0,buf[1];
  Int_t nbuf;

  for(nc = 0;nc<fNoComp;nc++)
    {
    
      // retrive material constants
      
      gMC->Gfmate((*fIdmate)[fMixtComp[nc]],namate,a,z,rho,X0,absl,buf,nbuf);

      amat[nc] = a;
      zmat[nc] = z;

      Int_t nnc = (fMixtComp[nc]>=20) ? fMixtComp[nc]%20 : fMixtComp[nc]%10;
 
      am += fMixtProp[nc]*((fMixtComp[nc]>=20) ? a_pure[nnc] : amol[nnc]); 
      density += fMixtProp[nc]*rho;  // density of the mixture
      
    }

  // mixture proportions by weight!

  for(nc = 0;nc<fNoComp;nc++)
    {

      Int_t nnc = (fMixtComp[nc]>=20) ? fMixtComp[nc]%20 : fMixtComp[nc]%10;

      wmat[nc] = fMixtProp[nc]*((fMixtComp[nc]>=20) ? a_pure[nnc] : amol[nnc])/am;

    }  
  
  AliMixture(31,"Drift gas 1",amat,zmat,density,fNoComp,wmat);
  AliMixture(32,"Drift gas 2",amat,zmat,density,fNoComp,wmat);
  AliMixture(33,"Drift gas 3",amat,zmat,density,fNoComp,wmat); 

  AliMedium(2, "Drift gas 1", 31, 0, ISXFLD, SXMGMX, 10., 999.,.1, .001, .001);
  AliMedium(3, "Drift gas 2", 32, 0, ISXFLD, SXMGMX, 10., 999.,.1, .001, .001);
  AliMedium(4, "Drift gas 3", 33, 1, ISXFLD, SXMGMX, 10., 999.,.1, .001, .001);

  // Air 

  AliMaterial(24, "Air", 14.61, 7.3, .001205, 30420., 67500.);

  AliMedium(24, "Air", 24, 0, ISXFLD, SXMGMX, 10., .1, .1, .1, .1);

  //----------------------------------------------------------------------
  //               solid materials
  //----------------------------------------------------------------------

  // Al

  AliMaterial(30, "Al", 26.98, 13., 2.7, 8.9, 37.2);

  AliMedium(0, "Al",30, 0, ISXFLD, SXMGMX, 10., .1, .1, .1,   .1);

  // Si

  AliMaterial(31, "Si", 28.086, 14.,2.33, 9.36, 999.);

  AliMedium(7, "Al",31, 0, ISXFLD, SXMGMX, 10., .1, .1, .1,   .1);
  

  // Mylar C5H4O2

  amat[0]=12.011;
  amat[1]=1.;
  amat[2]=15.9994;

  zmat[0]=6.;
  zmat[1]=1.;
  zmat[2]=8.;

  wmat[0]=5.;
  wmat[1]=4.;
  wmat[2]=2.; 

  density = 1.39;
  
  AliMixture(32, "Mylar",amat,zmat,density,-3,wmat);

  AliMedium(5, "Mylar",32, 0, ISXFLD, SXMGMX, 10., .1, .1, .001, .01);




  // Carbon (normal)

  AliMaterial(33,"C normal",12.011,6.,2.265,18.8,999.);

  AliMedium(6,"C normal",33,0, ISXFLD, SXMGMX, 10., .1, .1, .001, .01);

  // G10 for inner and outr field cage
  // G10 is 60% SiO2 + 40% epoxy, right now I use A and Z for SiO2

  Float_t rhoFactor;

  amat[0]=28.086;
  amat[1]=15.9994;

  zmat[0]=14.;
  zmat[1]=8.;

  wmat[0]=1.;
  wmat[1]=2.;

  density = 1.7;
  

  AliMixture(34,"G10 aux.",amat,zmat,density,-2,wmat);


  gMC->Gfmate((*fIdmate)[34],namate,a,z,rho,X0,absl,buf,nbuf);

  Float_t thickX0 = 0.0052; // field cage in X0 units
  
  Float_t thick = 2.; // in cm

  X0=19.4; // G10 

  rhoFactor = X0*thickX0/thick;
  density = rho*rhoFactor;

  AliMaterial(35,"G10-fc",a,z,density,999.,999.);

  AliMedium(8,"G10-fc",35,0, ISXFLD, SXMGMX, 10., .1, .1, .001, .01);

  thickX0 = 0.0027; // inner vessel (eta <0.9)
  thick=0.5;
  rhoFactor = X0*thickX0/thick;
  density = rho*rhoFactor;

  AliMaterial(36,"G10-iv",a,z,density,999.,999.);  

  AliMedium(9,"G10-iv",36,0, ISXFLD, SXMGMX, 10., .1, .1, .001, .01);

  //  Carbon fibre  
  
  gMC->Gfmate((*fIdmate)[33],namate,a,z,rho,X0,absl,buf,nbuf);

  thickX0 = 0.0133; // outer vessel
  thick=3.0;
  rhoFactor = X0*thickX0/thick;
  density = rho*rhoFactor;


  AliMaterial(37,"C-ov",a,z,density,999.,999.);

  AliMedium(10,"C-ov",37,0, ISXFLD, SXMGMX, 10., .1, .1, .001, .01);  

  thickX0=0.015; // inner vessel (cone, eta > 0.9)
  thick=1.5;
  rhoFactor = X0*thickX0/thick;
  density = rho*rhoFactor;

  AliMaterial(38,"C-ivc",a,z,density,999.,999.);

  AliMedium(11,"C-ivc",38,0, ISXFLD, SXMGMX, 10., .1, .1, .001, .01);

  //

  AliMedium(12,"CO2",10,0, ISXFLD, SXMGMX, 10., 999.,.1, .001, .001);
    


}

//_____________________________________________________________________________
struct Bin {
   const AliTPCdigit *dig;
   int idx;
   Bin() {dig=0; idx=-1;}
};

struct PreCluster : public AliTPCcluster {
  const AliTPCdigit* summit; //pointer to the maximum digit of this precluster
  int idx;                   //index in AliTPC::fClusters
  int npeaks;                //number of peaks in this precluster
  int ndigits;               //number of digits in this precluster
  PreCluster();
};
PreCluster::PreCluster() : AliTPCcluster() {npeaks=ndigits=0;}

//_____________________________________________________________________________
static void FindPreCluster(int i,int j,int maxj,Bin *bins,PreCluster &c) 
{
  //-----------------------------------------------------------------
  // This function looks for "preclusters".
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  Bin& b=bins[i*maxj+j];
  double q=(double)TMath::Abs(b.dig->fSignal);

  if (b.idx >= 0 && b.idx != c.idx) {
    c.idx=b.idx;
    c.npeaks++;
  }
  
  if (q > TMath::Abs(c.summit->fSignal)) c.summit=b.dig;
  
  c.fY += i*q;
  c.fZ += j*q;
  c.fSigmaY2 += i*i*q;
  c.fSigmaZ2 += j*j*q;
  c.fQ += q;
  c.ndigits++;

  b.dig = 0;  b.idx = c.idx;
  
  if (bins[(i-1)*maxj+j].dig) FindPreCluster(i-1,j,maxj,bins,c);
  if (bins[i*maxj+(j-1)].dig) FindPreCluster(i,j-1,maxj,bins,c);
  if (bins[(i+1)*maxj+j].dig) FindPreCluster(i+1,j,maxj,bins,c);
  if (bins[i*maxj+(j+1)].dig) FindPreCluster(i,j+1,maxj,bins,c);
  
}

//_____________________________________________________________________________
void AliTPC::Digits2Clusters()
{
  //-----------------------------------------------------------------
  // This is a simple cluster finder.
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  AliTPCParam *par = &(fDigParam->GetParam());
  
  int inp=par->GetNPads(0,                  par->GetNRowLow()-1);
  int onp=par->GetNPads(par->GetNSector()-1,par->GetNRowUp() -1);
  const int MAXY=(inp>onp) ? inp+2 : onp+2;
  const int MAXTBKT=int((z_end+6*par->GetZSigma())/par->GetZWidth())+1;
  const int MAXZ=MAXTBKT+2;
  const int THRESHOLD=20;
  
  TTree *t=(TTree*)gDirectory->Get("TreeD0_Param1");
  t->GetBranch("Digits")->SetAddress(&fDigits);
  Int_t sectors_by_rows=(Int_t)t->GetEntries();
  
  int ncls=0;
  
  for (Int_t n=0; n<sectors_by_rows; n++) {
    if (!t->GetEvent(n)) continue;
    Bin *bins=new Bin[MAXY*MAXZ];
    AliTPCdigit *dig=(AliTPCdigit*)fDigits->UncheckedAt(0);
    int sec=dig->fSector, row=dig->fPadRow;
    int ndigits=fDigits->GetEntriesFast();
    
    int npads, sign;
    {
       int nis=par->GetNInnerSector(), nos=par->GetNOuterSector();
       if (sec < nis) {
          npads = par->GetNPadsLow(row);
          sign = (sec < nis/2) ? 1 : -1;
       } else {
          npads = par->GetNPadsUp(row);
          sign = ((sec-nis) < nos/2) ? 1 : -1;
       }
    }

    int ndig;
    for (ndig=0; ndig<ndigits; ndig++) {
      dig=(AliTPCdigit*)fDigits->UncheckedAt(ndig);
      int i=dig->fPad+1, j=dig->fTime+1;
      if (i > npads) {
         cerr<<"AliTPC::Digits2Clusters error: pad number is out of range ! ";
         cerr<<i<<' '<<npads<<endl; 
         continue;
      }
      if (j > MAXTBKT) {
         cerr<<"AliTPC::Digits2Clusters error: time bucket is out of range ! ";
         cerr<<j<<' '<<MAXTBKT<<endl; 
         continue;
      }
      if (dig->fSignal >= THRESHOLD) bins[i*MAXZ+j].dig=dig;
      if (i==1 || i==npads || j==1 || j==MAXTBKT) dig->fSignal*=-1;
    }

    int ncl=0;
    int i,j;
    
    for (i=1; i<MAXY-1; i++) {
      for (j=1; j<MAXZ-1; j++) {
	if (bins[i*MAXZ+j].dig == 0) continue;
	PreCluster c; c.summit=bins[i*MAXZ+j].dig; c.idx=ncls;
	FindPreCluster(i, j, MAXZ, bins, c);
	c.fY /= c.fQ;
	c.fZ /= c.fQ;

        double s2 = c.fSigmaY2/c.fQ - c.fY*c.fY;
        c.fSigmaY2 = s2 + 1./12.;
        c.fSigmaY2 *= par->GetPadPitchWidth()*par->GetPadPitchWidth();
        if (s2 != 0.) c.fSigmaY2 *= 0.17;

        s2 = c.fSigmaZ2/c.fQ - c.fZ*c.fZ;
        c.fSigmaZ2 = s2 + 1./12.;
        c.fSigmaZ2 *= par->GetZWidth()*par->GetZWidth();
        if (s2 != 0.) c.fSigmaZ2 *= 0.41;

	c.fY = (c.fY - 0.5 - 0.5*npads)*par->GetPadPitchWidth();
	c.fZ = par->GetZWidth()*c.fZ; 
	c.fZ -= 3.*par->GetZSigma(); // PASA delay 
        c.fZ = sign*(z_end - c.fZ);

	c.fSector=sec;
	c.fPadRow=row;
	c.fTracks[0]=c.summit->fTracks[0];
	c.fTracks[1]=c.summit->fTracks[1];
	c.fTracks[2]=c.summit->fTracks[2];

        if (c.summit->fSignal<0) {
	  c.fSigmaY2 *= 25.;
	  c.fSigmaZ2 *= 4.;
	}
	
	AddCluster(c); ncls++; ncl++;
      }
    }
        
    for (ndig=0; ndig<ndigits; ndig++) {
      dig=(AliTPCdigit*)fDigits->UncheckedAt(ndig);
      int i=dig->fPad+1, j=dig->fTime+1;
      if (i > npads) {
         cerr<<"AliTPC::Digits2Clusters error: pad number is out of range ! ";
         cerr<<i<<' '<<npads<<endl; 
         continue;
      }
      if (j > MAXTBKT) {
         cerr<<"AliTPC::Digits2Clusters error: time bucket is out of range ! ";
         cerr<<j<<' '<<MAXTBKT<<endl; 
         continue;
      }
      if (TMath::Abs(dig->fSignal)>=par->GetZeroSup()) bins[i*MAXZ+j].dig=dig;
    }
    
    for (i=1; i<MAXY-1; i++) {
      for (j=1; j<MAXZ-1; j++) {
	if (bins[i*MAXZ+j].dig == 0) continue;
	PreCluster c; c.summit=bins[i*MAXZ+j].dig; c.idx=ncls;
	FindPreCluster(i, j, MAXZ, bins, c);
	if (c.ndigits < 2) continue; //noise cluster
        if (c.npeaks>1) continue;    //overlapped cluster
	c.fY /= c.fQ;
	c.fZ /= c.fQ;

        double s2 = c.fSigmaY2/c.fQ - c.fY*c.fY;
        c.fSigmaY2 = s2 + 1./12.;
        c.fSigmaY2 *= par->GetPadPitchWidth()*par->GetPadPitchWidth();
        if (s2 != 0.) c.fSigmaY2 *= 0.04;

        s2 = c.fSigmaZ2/c.fQ - c.fZ*c.fZ;
        c.fSigmaZ2 = s2 + 1./12.;
        c.fSigmaZ2 *= par->GetZWidth()*par->GetZWidth();
        if (s2 != 0.) c.fSigmaZ2 *= 0.10;

	c.fY = (c.fY - 0.5 - 0.5*npads)*par->GetPadPitchWidth();
	c.fZ = par->GetZWidth()*c.fZ; 
	c.fZ -= 3.*par->GetZSigma(); // PASA delay 
        c.fZ = sign*(z_end - c.fZ);
	
	c.fSector=sec;
	c.fPadRow=row;
	c.fTracks[0]=c.summit->fTracks[0];
	c.fTracks[1]=c.summit->fTracks[1];
	c.fTracks[2]=c.summit->fTracks[2];
	
        if (c.summit->fSignal<0) {
	  c.fSigmaY2 *= 25.;
	  c.fSigmaZ2 *= 4.;
	}

	if (c.npeaks==0) {AddCluster(c); ncls++; ncl++;}
	else {
	  new ((*fClusters)[c.idx]) AliTPCcluster(c);
	}
      }
    }
    
    cerr<<"sector, row, digits, clusters: "
	<<sec<<' '<<row<<' '<<ndigits<<' '<<ncl<<"                  \r";
    
    fDigits->Clear();
  
    delete[] bins;  
  }
}

//_____________________________________________________________________________
void AliTPC::ElDiff(Float_t *xyz)
{
  //--------------------------------------------------
  // calculates the diffusion of a single electron
  //--------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------
  AliTPCParam * fTPCParam = &(fDigParam->GetParam());
  Float_t driftl;
  
  Float_t z0=xyz[2];

  driftl=z_end-TMath::Abs(xyz[2]);

  if(driftl<0.01) driftl=0.01;

  // check the attachment

  driftl=TMath::Sqrt(driftl);

  //  Float_t sig_t = driftl*diff_t;
  //Float_t sig_l = driftl*diff_l;
  Float_t sig_t = driftl*fTPCParam->GetDiffT();
  Float_t sig_l = driftl*fTPCParam->GetDiffL();
  xyz[0]=gRandom->Gaus(xyz[0],sig_t);
  xyz[1]=gRandom->Gaus(xyz[1],sig_t);
  xyz[2]=gRandom->Gaus(xyz[2],sig_l);
  
  if (TMath::Abs(xyz[2])>z_end){
    xyz[2]=TMath::Sign(z_end,z0);
  }
  if(xyz[2]*z0 < 0.){
    Float_t eps = 0.0001;
    xyz[2]=TMath::Sign(eps,z0);
  } 
}

//_____________________________________________________________________________
void AliTPC::Hits2Clusters()
{
  //--------------------------------------------------------
  // TPC simple cluster generator from hits
  // obtained from the TPC Fast Simulator
  // The point errors are taken from the parametrization
  //--------------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  AliTPCParam * fTPCParam = &(fDigParam->GetParam());
  Float_t sigma_rphi,sigma_z,cl_rphi,cl_z;
  //
  TParticle *particle; // pointer to a given particle
  AliTPChit *tpcHit; // pointer to a sigle TPC hit
  TClonesArray *Particles; //pointer to the particle list
  Int_t sector,nhits;
  Int_t ipart;
  Float_t xyz[5];
  Float_t pl,pt,tanth,rpad,ratio;
  Float_t cph,sph;
  
  //---------------------------------------------------------------
  //  Get the access to the tracks 
  //---------------------------------------------------------------
  
  TTree *TH = gAlice->TreeH();
  Stat_t ntracks = TH->GetEntries();
  Particles=gAlice->Particles();
  
  //------------------------------------------------------------
  // Loop over all sectors (72 sectors)
  // Sectors 0-35 are lower sectors, 0-17 z>0, 18-35 z<0
  // Sectors 36-71 are upper sectors, 36-53 z>0, 54-71 z<0
  //
  // First cluster for sector 0 starts at "0"
  //------------------------------------------------------------
  
  
  //fClustersIndex[0] = 0;
  
  //
  int Nsectors=fDigParam->GetParam().GetNSector();
  for(Int_t isec=0; isec<Nsectors; isec++){
    //MI change
    fTPCParam->AdjustAngles(isec,cph,sph);
    
    //------------------------------------------------------------
    // Loop over tracks
    //------------------------------------------------------------
    
    for(Int_t track=0;track<ntracks;track++){
      ResetHits();
      TH->GetEvent(track);
      //
      //  Get number of the TPC hits and a pointer
      //  to the particles
      //
      nhits=fHits->GetEntriesFast();
      //
      // Loop over hits
      //
      for(Int_t hit=0;hit<nhits;hit++){
	tpcHit=(AliTPChit*)fHits->UncheckedAt(hit);
	sector=tpcHit->fSector; // sector number
	if(sector != isec) continue; //terminate iteration
	ipart=tpcHit->fTrack;
	particle=(TParticle*)Particles->UncheckedAt(ipart);
	pl=particle->Pz();
	pt=particle->Pt();
	if(pt < 1.e-9) pt=1.e-9;
	tanth=pl/pt;
	tanth = TMath::Abs(tanth);
	rpad=TMath::Sqrt(tpcHit->fX*tpcHit->fX + tpcHit->fY*tpcHit->fY);
	ratio=0.001*rpad/pt; // pt must be in MeV/c - historical reason
	
	//   space-point resolutions
	
	sigma_rphi=SigmaY2(rpad,tanth,pt);
	sigma_z   =SigmaZ2(rpad,tanth   );
	
	//   cluster widths
	
	cl_rphi=ac_rphi-bc_rphi*rpad*tanth+cc_rphi*ratio*ratio;
	cl_z=ac_z-bc_z*rpad*tanth+cc_z*tanth*tanth;
	
	// temporary protection
	
	if(sigma_rphi < 0.) sigma_rphi=0.4e-3;
	if(sigma_z < 0.) sigma_z=0.4e-3;
	if(cl_rphi < 0.) cl_rphi=2.5e-3;
	if(cl_z < 0.) cl_z=2.5e-5;
	
	//
	// smearing --> rotate sectors firstly,
	// then the inaccuracy in a X-Y plane is only along Y (pad row)!
	//
	Float_t xprim= tpcHit->fX*cph + tpcHit->fY*sph;
	Float_t yprim=-tpcHit->fX*sph + tpcHit->fY*cph;
	xyz[0]=gRandom->Gaus(yprim,TMath::Sqrt(sigma_rphi));   // y
        Double_t alpha=(sector < fTPCParam->GetNInnerSector()) ?
        fTPCParam->GetInnerAngle() : fTPCParam->GetOuterAngle();
	if (TMath::Abs(xyz[0]/xprim) > TMath::Tan(0.5*alpha)) xyz[0]=yprim;
	xyz[1]=gRandom->Gaus(tpcHit->fZ,TMath::Sqrt(sigma_z)); // z 
        if (TMath::Abs(xyz[1]) > 250) xyz[1]=tpcHit->fZ;
	xyz[2]=tpcHit->fQ+1;// q; let it be not equal to zero (Y.Belikov)
	xyz[3]=sigma_rphi;                                     // fSigmaY2
	xyz[4]=sigma_z;                                        // fSigmaZ2
		
	// and finally add the cluster
	Int_t tracks[5]={tpcHit->fTrack, -1, -1, sector, tpcHit->fPadRow};
	AddCluster(xyz,tracks);
	
      } // end of loop over hits
    }   // end of loop over tracks 
    
    //fClustersIndex[isec] = fNclusters; // update clusters index
    
  } // end of loop over sectors
  
  //fClustersIndex[fNsectors]--; // set end of the clusters buffer
  
}


void AliTPC::Hits2Digits()  
{ 

 //----------------------------------------------------
  // Loop over all sectors (72 sectors)
  // Sectors 0-35 are lower sectors, 0-17 z>0, 18-35 z<0
  // Sectors 36-71 are upper sectors, 36-53 z>0, 54-71 z<0
  //----
  int Nsectors=fDigParam->GetParam().GetNSector();
  for(Int_t isec=0;isec<Nsectors;isec++)  Hits2DigitsSector(isec);
}


//_____________________________________________________________________________
void AliTPC::Hits2DigitsSector(Int_t isec)
{
  //-------------------------------------------------------------------
  // TPC conversion from hits to digits.
  //------------------------------------------------------------------- 

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  //-------------------------------------------------------
  //  Get the access to the track hits
  //-------------------------------------------------------

  AliTPCParam * fTPCParam = &(fDigParam->GetParam());
  TTree *TH = gAlice->TreeH(); // pointer to the hits tree
  Stat_t ntracks = TH->GetEntries();

  if( ntracks > 0){

  //------------------------------------------- 
  //  Only if there are any tracks...
  //-------------------------------------------

    
    // TObjArrays for three neighouring pad-rows

    TObjArray **rowTriplet = new TObjArray* [3]; 
    
    // TObjArray-s for each pad-row

    TObjArray **row;
      
    for (Int_t trip=0;trip<3;trip++){  
      rowTriplet[trip]=new TObjArray;
    }


    
      printf("*** Processing sector number %d ***\n",isec);

      Int_t nrows =fTPCParam->GetNRow(isec);

      row= new TObjArray* [nrows];
    
      MakeSector(isec,nrows,TH,ntracks,row);

      //--------------------------------------------------------
      //   Digitize this sector, row by row
      //   row[i] is the pointer to the TObjArray of TVectors,
      //   each one containing electrons accepted on this
      //   row, assigned into tracks
      //--------------------------------------------------------

      Int_t i;

      for (i=0;i<nrows;i++){

	// Triplets for i = 0 and i=1 are identical!
	// The same for i = nrows-1 and nrows!

        if(i != 1 && i != nrows-1){
	   MakeTriplet(i,rowTriplet,row);
	 }

	    DigitizeRow(i,isec,rowTriplet);

	    fDigParam->Fill();

            Int_t ndig=fDigParam->GetArray()->GetEntriesFast();

            printf("*** Sector, row, digits %d %d %d ***\n",isec,i,ndig);

	    ResetDigits(); // reset digits for this row after storing them
             
       } // end of the sector digitization
     
       // delete the last triplet

       for (i=0;i<3;i++) rowTriplet[i]->Delete();
          
       delete [] row; // delete the array of pointers to TObjArray-s
        
  } // ntracks >0
} // end of Hits2Digits
//_____________________________________________________________________________
void AliTPC::MakeTriplet(Int_t row,
                         TObjArray **rowTriplet, TObjArray **prow)
{
  //------------------------------------------------------------------
  //  Makes the "triplet" of the neighbouring pad-row for the
  //  digitization including the cross-talk between the pad-rows
  //------------------------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  AliTPCParam * fTPCParam = &(fDigParam->GetParam());
  Float_t gasgain = fTPCParam->GetGasGain();
  Int_t nTracks[3];

  Int_t nElements,nElectrons;

  TVector *pv;
  TVector *tv;

  //-------------------------------------------------------------------
  // pv is an "old" track, i.e. label + triplets of (x,y,z) 
  // for each electron
  //
  //-------------------------------------------------------------------


  Int_t i1,i2;
  Int_t nel,nt;

  if(row == 0 || row == 1){

  // create entire triplet for the first AND the second row

    nTracks[0] = prow[0]->GetEntries();
    nTracks[1] = prow[1]->GetEntries();
    nTracks[2] = prow[2]->GetEntries();

    for(i1=0;i1<3;i1++){
      nt = nTracks[i1]; // number of tracks for this row

      for(i2=0;i2<nt;i2++){
        pv = (TVector*)prow[i1]->At(i2);
        TVector &v1 = *pv;
        nElements = pv->GetNrows(); 
        nElectrons = (nElements-1)/3;

        tv = new TVector(4*nElectrons+1); // create TVector for a modified track
        TVector &v2 = *tv;
        v2(0)=v1(0); //track label

        for(nel=0;nel<nElectrons;nel++){        
          Int_t idx1 = nel*3;
          Int_t idx2 = nel*4;
       	  // Avalanche, including fluctuations
          Int_t aval = (Int_t) (-gasgain*TMath::Log(gRandom->Rndm()));
          v2(idx2+1)= v1(idx1+1);
          v2(idx2+2)= v1(idx1+2);
          v2(idx2+3)= v1(idx1+3);
          v2(idx2+4)= (Float_t)aval; // in number of electrons!        
	} // end of loop over electrons
	//
	//  Add this track to a row 
	//

        rowTriplet[i1]->Add(tv); 


      } // end of loop over tracks for this row

      prow[i1]->Delete(); // remove "old" tracks
      delete prow[i1]; // delete  TObjArray for this row
      prow[i1]=0; // set pointer to NULL

    } // end of loop over row triplets


  }
  else{
   
    rowTriplet[0]->Delete(); // remove old lower row

    nTracks[0]=rowTriplet[1]->GetEntries(); // previous middle row
    nTracks[1]=rowTriplet[2]->GetEntries(); // previous upper row
    nTracks[2]=prow[row+1]->GetEntries(); // next row
    

    //------------------------------------------- 
    //  shift new tracks downwards
    //-------------------------------------------

    for(i1=0;i1<nTracks[0];i1++){
      pv=(TVector*)rowTriplet[1]->At(i1);
      rowTriplet[0]->Add(pv); 
    }       

    rowTriplet[1]->Clear(); // leave tracks on the heap!!!

    for(i1=0;i1<nTracks[1];i1++){
      pv=(TVector*)rowTriplet[2]->At(i1);
      rowTriplet[1]->Add(pv);
    }

    rowTriplet[2]->Clear(); // leave tracks on the heap!!!

    //---------------------------------------------
    //  Create new upper row
    //---------------------------------------------

    

    for(i1=0;i1<nTracks[2];i1++){
        pv = (TVector*)prow[row+1]->At(i1);
        TVector &v1 = *pv;
        nElements = pv->GetNrows();
        nElectrons = (nElements-1)/3;

        tv = new TVector(4*nElectrons+1); // create TVector for a modified track
        TVector &v2 = *tv;
        v2(0)=v1(0); //track label

        for(nel=0;nel<nElectrons;nel++){
        
          Int_t idx1 = nel*3;
          Int_t idx2 = nel*4;
	  // Avalanche, including fluctuations
          Int_t aval = (Int_t) 
	    (-gasgain*TMath::Log(gRandom->Rndm()));
          
          v2(idx2+1)= v1(idx1+1); 
          v2(idx2+2)= v1(idx1+2);
          v2(idx2+3)= v1(idx1+3);
          v2(idx2+4)= (Float_t)aval; // in number of electrons!        
	} // end of loop over electrons

          rowTriplet[2]->Add(tv);
     
    } // end of loop over tracks
         
    prow[row+1]->Delete(); // delete tracks for this row
    delete prow[row+1];  // delete TObjArray for this row
    prow[row+1]=0; // set a pointer to NULL

  }

}  // end of MakeTriplet
//_____________________________________________________________________________
void AliTPC::ExB(Float_t *xyz)
{
  //------------------------------------------------
  //  ExB at the wires and wire number calulation
  //------------------------------------------------
  
  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------
   AliTPCParam * fTPCParam = &(fDigParam->GetParam());

   Float_t x1=xyz[0];
   fTPCParam->GetWire(x1);        //calculate nearest wire position
   Float_t dx=xyz[0]-x1;
   xyz[1]+=dx*fTPCParam->GetOmegaTau();

} // end of ExB
//_____________________________________________________________________________
void AliTPC::DigitizeRow(Int_t irow,Int_t isec,TObjArray **rowTriplet)
{
  //-----------------------------------------------------------
  // Single row digitization, coupling from the neighbouring
  // rows taken into account
  //-----------------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------
 

  AliTPCParam * fTPCParam = &(fDigParam->GetParam());
  Float_t chipgain= fTPCParam->GetChipGain();
  Float_t zerosup = fTPCParam->GetZeroSup();
  Int_t nrows =fTPCParam->GetNRow(isec);
  const int MAXTBKT=
  int((z_end+6*fTPCParam->GetZSigma())/fTPCParam->GetZWidth())+1;
  
  Int_t nTracks[3];
  Int_t n_of_pads[3];
  Int_t IndexRange[4];
  Int_t i1;
  Int_t iFlag; 

  //
  // iFlag = 0 -> inner row, iFlag = 1 -> the middle one, iFlag = 2 -> the outer one
  //

  nTracks[0]=rowTriplet[0]->GetEntries(); // lower row
  nTracks[1]=rowTriplet[1]->GetEntries(); // middle row
  nTracks[2]=rowTriplet[2]->GetEntries(); // upper row

    
  if(irow == 0){
    iFlag=0;
    n_of_pads[0]=fTPCParam->GetNPads(isec,0);
    n_of_pads[1]=fTPCParam->GetNPads(isec,1);
  }
  else if(irow == nrows-1){
     iFlag=2;
     n_of_pads[1]=fTPCParam->GetNPads(isec,irow-1);
     n_of_pads[2]=fTPCParam->GetNPads(isec,irow);
  }
  else {
    iFlag=1;
    for(i1=0;i1<3;i1++){
       n_of_pads[i1]=fTPCParam->GetNPads(isec,irow-1+i1);
    }
  }
 
  //
  //  Integrated signal for this row
  //  and a single track signal
  // 
   
  TMatrix *m1   = new TMatrix(0,n_of_pads[iFlag]-1,0,MAXTBKT-1); // integrated
  TMatrix *m2   = new TMatrix(0,n_of_pads[iFlag]-1,0,MAXTBKT-1); // single

  //

  TMatrix &Total  = *m1;

  //  Array of pointers to the label-signal list

  Int_t NofDigits = n_of_pads[iFlag]*MAXTBKT; // number of digits for this row

  Float_t  **pList = new Float_t* [NofDigits]; 

  Int_t lp;

  for(lp=0;lp<NofDigits;lp++)pList[lp]=0; // set all pointers to NULL

  //
  //  Straight signal and cross-talk, cross-talk is integrated over all
  //  tracks and added to the signal at the very end
  //
   

  for (i1=0;i1<nTracks[iFlag];i1++){

    m2->Zero(); // clear single track signal matrix
  
    Float_t TrackLabel = 
      GetSignal(rowTriplet[iFlag],i1,n_of_pads[iFlag],m2,m1,IndexRange); 

    GetList(TrackLabel,n_of_pads[iFlag],m2,IndexRange,pList);

  }

  // 
  //  Cross talk from the neighbouring pad-rows
  //

  TMatrix *m3 =  new TMatrix(0,n_of_pads[iFlag]-1,0,MAXTBKT-1); // cross-talk

  TMatrix &Cross = *m3;

  if(iFlag == 0){

    // cross-talk from the outer row only (first pad row)

    GetCrossTalk(0,rowTriplet[1],nTracks[1],n_of_pads,m3);

  }
  else if(iFlag == 2){

    // cross-talk from the inner row only (last pad row)

    GetCrossTalk(2,rowTriplet[1],nTracks[1],n_of_pads,m3);

  }
  else{

    // cross-talk from both inner and outer rows

    GetCrossTalk(3,rowTriplet[0],nTracks[0],n_of_pads,m3); // inner
    GetCrossTalk(4,rowTriplet[2],nTracks[2],n_of_pads,m3); //outer
  }

  Total += Cross; // add the cross-talk

  //
  //  Convert analog signal to ADC counts
  //
   
  Int_t tracks[3];
  Int_t digits[5];


  for(Int_t ip=0;ip<n_of_pads[iFlag];ip++){
    for(Int_t it=0;it<MAXTBKT;it++){

      Float_t q = Total(ip,it);

      Int_t gi =it*n_of_pads[iFlag]+ip; // global index

      q = gRandom->Gaus(q,fTPCParam->GetNoise()); // apply noise
      q *= (q_el*1.e15); // convert to fC
      q *= chipgain; // convert to mV   
      q *= (adc_sat/dyn_range); // convert to ADC counts  

      if(q <zerosup) continue; // do not fill zeros
      if(q > adc_sat) q = adc_sat;  // saturation

      //
      //  "real" signal or electronic noise (list = -1)?
      //    

      for(Int_t j1=0;j1<3;j1++){
        tracks[j1] = (pList[gi]) ?(Int_t)(*(pList[gi]+j1)) : -1;
      }

      digits[0]=isec;
      digits[1]=irow;
      digits[2]=ip;
      digits[3]=it;
      digits[4]= (Int_t)q;

      //  Add this digit

      AddDigit(tracks,digits);
    
    } // end of loop over time buckets
  }  // end of lop over pads 

  //
  //  This row has been digitized, delete nonused stuff
  //

  for(lp=0;lp<NofDigits;lp++){
    if(pList[lp]) delete [] pList[lp];
  }
  
  delete [] pList;

  delete m1;
  delete m2;
  delete m3;

} // end of DigitizeRow
//_____________________________________________________________________________
Float_t AliTPC::GetSignal(TObjArray *p1, Int_t ntr, Int_t np, TMatrix *m1, TMatrix *m2,
                          Int_t *IndexRange)
{

  //---------------------------------------------------------------
  //  Calculates 2-D signal (pad,time) for a single track,
  //  returns a pointer to the signal matrix and the track label 
  //  No digitization is performed at this level!!!
  //---------------------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  TVector *tv;
  AliTPCParam * fTPCParam = &(fDigParam->GetParam());
  AliTPCPRF2D * fPRF2D = &(fDigParam->GetPRF2D());
  AliTPCRF1D  * fRF    = &(fDigParam->GetRF()); 
  const int MAXTBKT=
  int((z_end+6*fTPCParam->GetZSigma())/fTPCParam->GetZWidth())+1;
 
  //to make the code faster we put parameters  to the stack

  Float_t zwidth  = fTPCParam->GetZWidth();
  Float_t zwidthm1  =1./zwidth;

  tv = (TVector*)p1->At(ntr); // pointer to a track
  TVector &v = *tv;
  
  Float_t label = v(0);

  Int_t CentralPad = (np-1)/2;
  Int_t PadNumber;
  Int_t nElectrons = (tv->GetNrows()-1)/4;
  Float_t range=((np-1)/2 + 0.5)*fTPCParam->GetPadPitchWidth(); // pad range

  range -= 0.5; // dead zone, 5mm from the edge, according to H.G. Fischer
  
  Float_t IneffFactor = 0.5; // inefficiency in the gain close to the edge, as above


  Float_t PadSignal[7]; // signal from a single electron

  TMatrix &signal = *m1;
  TMatrix &total = *m2;


  IndexRange[0]=9999; // min pad
  IndexRange[1]=-1; // max pad
  IndexRange[2]=9999; //min time
  IndexRange[3]=-1; // max time

  //
  //  Loop over all electrons
  //

  for(Int_t nel=0; nel<nElectrons; nel++){
   Int_t idx=nel*4;
   Float_t xwire = v(idx+1);
   Float_t y = v(idx+2);
   Float_t z = v(idx+3);


   Float_t absy=TMath::Abs(y);
   
   if(absy < 0.5*fTPCParam->GetPadPitchWidth()){
     PadNumber=CentralPad;
   }
   else if (absy < range){
     PadNumber=(Int_t) ((absy-0.5*fTPCParam->GetPadPitchWidth())/fTPCParam->GetPadPitchWidth()+1.);
     PadNumber=(Int_t) (TMath::Sign((Float_t)PadNumber, y)+CentralPad);
   }
   else continue; // electron out of pad-range , lost at the sector edge
    
   Float_t aval = (absy<range-0.5) ? v(idx+4):v(idx+4)*IneffFactor;
   

   Float_t dist = y - (Float_t)(PadNumber-CentralPad)*fTPCParam->GetPadPitchWidth();
   for (Int_t i=0;i<7;i++){
     PadSignal[i]=fPRF2D->GetPRF(-dist+(i-3)*fTPCParam->GetPadPitchWidth(),xwire)*aval;
     PadSignal[i] *= fTPCParam->GetPadCoupling();
   }

   Int_t  LeftPad = TMath::Max(0,PadNumber-3);
   Int_t  RightPad = TMath::Min(np-1,PadNumber+3);

   Int_t pmin=LeftPad-PadNumber+3; // lower index of the pad_signal vector
   Int_t pmax=RightPad-PadNumber+3; // upper index     
   
   Float_t z_drift = z*zwidthm1;
   Float_t z_offset = z_drift-(Int_t)z_drift;

   Int_t FirstBucket = (Int_t)z_drift; // numbering starts from "0"


   // loop over time bins (4 bins is enough - 3 sigma truncated Gaussian)

   for (Int_t i2=0;i2<4;i2++){          
     Int_t TrueTime = FirstBucket+i2; // current time bucket
     Float_t dz   = (Float_t(i2)+1.-z_offset)*zwidth; 
     Float_t ampl = fRF->GetRF(dz); 
     if( (TrueTime>MAXTBKT-1) ) break; // beyond the time range
     
     IndexRange[2]=TMath::Min(IndexRange[2],TrueTime); // min time
     IndexRange[3]=TMath::Max(IndexRange[3],TrueTime); // max time

     // loop over pads, from pmin to pmax
     for(Int_t i3=pmin;i3<=pmax;i3++){
       Int_t TruePad = LeftPad+i3-pmin;
       IndexRange[0]=TMath::Min(IndexRange[0],TruePad); // min pad
       IndexRange[1]=TMath::Max(IndexRange[1],TruePad); // max pad
       signal(TruePad,TrueTime)+=(PadSignal[i3]*ampl); // not converted to charge!!!
       total(TruePad,TrueTime)+=(PadSignal[i3]*ampl); // not converted to charge!!!
     } // end of pads loop
   }  // end of time loop
  } // end of loop over electrons

  return label; // returns track label when finished
}

//_____________________________________________________________________________
void AliTPC::GetList(Float_t label,Int_t np,TMatrix *m,Int_t *IndexRange,
                     Float_t **pList)
{
  //----------------------------------------------------------------------
  //  Updates the list of tracks contributing to digits for a given row
  //----------------------------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  TMatrix &signal = *m;

  // lop over nonzero digits

  for(Int_t it=IndexRange[2];it<IndexRange[3]+1;it++){
    for(Int_t ip=IndexRange[0];ip<IndexRange[1]+1;ip++){


        Int_t GlobalIndex = it*np+ip; // GlobalIndex starts from 0!
        
        if(!pList[GlobalIndex]){
        
          // 
	  // Create new list (6 elements - 3 signals and 3 labels),
	  // but only if the signal is > 0. 
	  //

          if(signal(ip,it)>0.){

          pList[GlobalIndex] = new Float_t [6];

	  // set list to -1 

          *pList[GlobalIndex] = -1.;
          *(pList[GlobalIndex]+1) = -1.;
          *(pList[GlobalIndex]+2) = -1.;
          *(pList[GlobalIndex]+3) = -1.;
          *(pList[GlobalIndex]+4) = -1.;
          *(pList[GlobalIndex]+5) = -1.;


          *pList[GlobalIndex] = label;
          *(pList[GlobalIndex]+3) = signal(ip,it);}
        }
        else{

	  // check the signal magnitude

          Float_t highest = *(pList[GlobalIndex]+3);
          Float_t middle = *(pList[GlobalIndex]+4);
          Float_t lowest = *(pList[GlobalIndex]+5);

	  //
	  //  compare the new signal with already existing list
	  //

          if(signal(ip,it)<lowest) continue; // neglect this track

	  //

          if (signal(ip,it)>highest){
            *(pList[GlobalIndex]+5) = middle;
            *(pList[GlobalIndex]+4) = highest;
            *(pList[GlobalIndex]+3) = signal(ip,it);

            *(pList[GlobalIndex]+2) = *(pList[GlobalIndex]+1);
            *(pList[GlobalIndex]+1) = *pList[GlobalIndex];
            *pList[GlobalIndex] = label;
	  }
          else if (signal(ip,it)>middle){
            *(pList[GlobalIndex]+5) = middle;
            *(pList[GlobalIndex]+4) = signal(ip,it);

            *(pList[GlobalIndex]+2) = *(pList[GlobalIndex]+1);
            *(pList[GlobalIndex]+1) = label;
	  }
          else{
            *(pList[GlobalIndex]+5) = signal(ip,it);
            *(pList[GlobalIndex]+2) = label;
	  }
        }

    } // end of loop over pads
  } // end of loop over time bins




}//end of GetList
//___________________________________________________________________
void AliTPC::MakeSector(Int_t isec,Int_t nrows,TTree *TH,
                        Stat_t ntracks,TObjArray **row)
{

  //-----------------------------------------------------------------
  // Prepares the sector digitization, creates the vectors of
  // tracks for each row of this sector. The track vector
  // contains the track label and the position of electrons.
  //-----------------------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  AliTPCParam * fTPCParam = &(fDigParam->GetParam());
  Int_t i;
  Float_t xyz[3]; 

  AliTPChit *tpcHit; // pointer to a sigle TPC hit    
 
  //----------------------------------------------
  // Create TObjArray-s, one for each row,
  // each TObjArray will store the TVectors
  // of electrons, one TVector per each track.
  //---------------------------------------------- 
    
  for(i=0; i<nrows; i++){
    row[i] = new TObjArray;
  }
  Int_t *n_of_electrons = new Int_t [nrows]; // electron counter for each row
  TVector **tr = new TVector* [nrows]; //pointers to the track vectors

  //--------------------------------------------------------------------
  //  Loop over tracks, the "track" contains the full history
  //--------------------------------------------------------------------

  Int_t previousTrack,currentTrack;
  previousTrack = -1; // nothing to store so far!

  for(Int_t track=0;track<ntracks;track++){

    ResetHits();

    TH->GetEvent(track); // get next track
    Int_t nhits = fHits->GetEntriesFast(); // get number of hits for this track

    if(nhits == 0) continue; // no hits in the TPC for this track

    //--------------------------------------------------------------
    //  Loop over hits
    //--------------------------------------------------------------

    for(Int_t hit=0;hit<nhits;hit++){

      tpcHit = (AliTPChit*)fHits->UncheckedAt(hit); // get a pointer to a hit
      
      Int_t sector=tpcHit->fSector; // sector number
      if(sector != isec) continue; //terminate iteration

	currentTrack = tpcHit->fTrack; // track number
        if(currentTrack != previousTrack){
                          
           // store already filled fTrack
              
	   for(i=0;i<nrows;i++){
             if(previousTrack != -1){
	       if(n_of_electrons[i]>0){
	         TVector &v = *tr[i];
		 v(0) = previousTrack;
                 tr[i]->ResizeTo(3*n_of_electrons[i]+1); // shrink if necessary
	         row[i]->Add(tr[i]);                     
	       }
               else{
                 delete tr[i]; // delete empty TVector
                 tr[i]=0;
	       }
	     }

             n_of_electrons[i]=0;
             tr[i] = new TVector(361); // TVectors for the next fTrack

	   } // end of loop over rows
	       
           previousTrack=currentTrack; // update track label 
	}
	   
	Int_t QI = (Int_t) (tpcHit->fQ); // energy loss (number of electrons)

       //---------------------------------------------------
       //  Calculate the electron attachment probability
       //---------------------------------------------------

        Float_t time = 1.e6*(z_end-TMath::Abs(tpcHit->fZ))/fTPCParam->GetDriftV(); 
	// in microseconds!	
	Float_t AttProb = fTPCParam->GetAttCoef()*
	  fTPCParam->GetOxyCont()*time; //  fraction! 
   
	//-----------------------------------------------
	//  Loop over electrons
	//-----------------------------------------------

        for(Int_t nel=0;nel<QI;nel++){
          // skip if electron lost due to the attachment
          if((gRandom->Rndm(0)) < AttProb) continue; // electron lost!
	  xyz[0]=tpcHit->fX;
	  xyz[1]=tpcHit->fY;
	  xyz[2]=tpcHit->fZ;
	  ElDiff(xyz); // Appply the diffusion
	  Int_t row_number;
	  fTPCParam->XYZtoCRXYZ(xyz,isec,row_number,3);

	  //transform position to local coordinates
	  //option 3 means that we calculate x position relative to 
	  //nearest pad row 

	  if ((row_number<0)||row_number>=fTPCParam->GetNRow(isec)) continue;
	  ExB(xyz); // ExB effect at the sense wires
	  n_of_electrons[row_number]++;	  
	  //----------------------------------
	  // Expand vector if necessary
	  //----------------------------------
	  if(n_of_electrons[row_number]>120){
	    Int_t range = tr[row_number]->GetNrows();
	    if(n_of_electrons[row_number] > (range-1)/3){
	      tr[row_number]->ResizeTo(range+150); // Add 50 electrons
	    }
	  }
	  
	  TVector &v = *tr[row_number];
	  Int_t idx = 3*n_of_electrons[row_number]-2;

	  v(idx)=  xyz[0];   // X
	  v(idx+1)=xyz[1];   // Y (along the pad-row)
          v(idx+2)=xyz[2];   // Z
	    
	} // end of loop over electrons
        
      } // end of loop over hits
    } // end of loop over tracks

    //
    //   store remaining track (the last one) if not empty
    //

     for(i=0;i<nrows;i++){
       if(n_of_electrons[i]>0){
          TVector &v = *tr[i];
	  v(0) = previousTrack;
          tr[i]->ResizeTo(3*n_of_electrons[i]+1); // shrink if necessary
	  row[i]->Add(tr[i]);  
	}
	else{
          delete tr[i];
          tr[i]=0;
	}  
      }  

          delete [] tr;
          delete [] n_of_electrons; 

} // end of MakeSector
//_____________________________________________________________________________
void AliTPC::GetCrossTalk (Int_t iFlag,TObjArray *p,Int_t ntracks,Int_t *npads,
                           TMatrix *m)
{

  //-------------------------------------------------------------
  // Calculates the cross-talk from one row (inner or outer)
  //-------------------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  //
  // iFlag=2 & 3 --> cross-talk from the inner row
  // iFlag=0 & 4 --> cross-talk from the outer row
  //
  AliTPCParam * fTPCParam = &(fDigParam->GetParam());
  AliTPCPRF2D * fPRF2D = &(fDigParam->GetPRF2D());
  AliTPCRF1D  * fRF    = &(fDigParam->GetRF()); 
  const int MAXTBKT=
  int((z_end+6*fTPCParam->GetZSigma())/fTPCParam->GetZWidth())+1;
 
  //to make code faster

  Float_t zwidth  = fTPCParam->GetZWidth();
  Float_t zwidthm1  =1/fTPCParam->GetZWidth();

 Int_t PadNumber;
 Float_t xwire;

 Int_t nPadsSignal; // for this pads the signal is calculated
 Float_t range;     // sense wire range
 Int_t nPadsDiff;

 Float_t IneffFactor=0.5; // gain inefficiency close to the edges

 if(iFlag == 0){   
   // 1-->0
   nPadsSignal = npads[1];
   range = ((npads[1]-1)/2 + 0.5)*fTPCParam->GetPadPitchWidth();  
   nPadsDiff = (npads[1]-npads[0])/2;
 }  
 else if (iFlag == 2){
   // 1-->2
   nPadsSignal = npads[2];
   range = ((npads[1]-1)/2 + 0.5)*fTPCParam->GetPadPitchWidth();
   nPadsDiff = 0;
 }
 else if (iFlag == 3){
   // 0-->1
   nPadsSignal = npads[1];
   range = ((npads[0]-1)/2 + 0.5)*fTPCParam->GetPadPitchWidth();
   nPadsDiff = 0;
 }
 else{
   // 2-->1
   nPadsSignal = npads[2];
   range = ((npads[2]-1)/2 + 0.5)*fTPCParam->GetPadPitchWidth();
   nPadsDiff = (npads[2]-npads[1])/2;
 }

 range-=0.5; // dead zone close to the edges

 TVector *tv; 
 TMatrix &signal = *m;

 Int_t CentralPad = (nPadsSignal-1)/2;
 Float_t PadSignal[7]; // signal from a single electron
 // Loop over tracks
 for(Int_t nt=0;nt<ntracks;nt++){
   tv=(TVector*)p->At(nt); // pointer to a track
   TVector &v = *tv;
   Int_t nElectrons = (tv->GetNrows()-1)/4;
   // Loop over electrons
   for(Int_t nel=0; nel<nElectrons; nel++){
     Int_t idx=nel*4;
     xwire=v(idx+1);
 
     if (iFlag==0) xwire+=fTPCParam->GetPadPitchLength();
     if (iFlag==2)  xwire-=fTPCParam->GetPadPitchLength();
     if (iFlag==3)  xwire-=fTPCParam->GetPadPitchLength();
     if (iFlag==4)  xwire+=fTPCParam->GetPadPitchLength();  
   
     //  electron acceptance for the cross-talk (only the closest wire)  

     Float_t dxMax = fTPCParam->GetPadPitchLength()*0.5+fTPCParam->GetWWPitch();
     if(TMath::Abs(xwire)>dxMax) continue;

     Float_t y = v(idx+2);
     Float_t z = v(idx+3);
     Float_t absy=TMath::Abs(y);

     if(absy < 0.5*fTPCParam->GetPadPitchWidth()){
       PadNumber=CentralPad;
     }
     else if (absy < range){
       PadNumber=(Int_t) ((absy-0.5*fTPCParam->GetPadPitchWidth())/fTPCParam->GetPadPitchWidth() +1.);
       PadNumber=(Int_t) (TMath::Sign((Float_t)PadNumber, y)+CentralPad);
     }
     else continue; // electron out of sense wire range, lost at the sector edge

     Float_t aval = (absy<range-0.5) ? v(idx+4):v(idx+4)*IneffFactor;

     Float_t dist = y - (Float_t)(PadNumber-CentralPad)*fTPCParam->GetPadPitchWidth();
       
     for (Int_t i=0;i<7;i++){
       PadSignal[i]=fPRF2D->GetPRF(-dist+(i-3)*fTPCParam->GetPadPitchWidth(),xwire)*aval;

       PadSignal[i] *= fTPCParam->GetPadCoupling();
     }
     // real pad range

     Int_t  LeftPad = TMath::Max(0,PadNumber-3);
     Int_t  RightPad = TMath::Min(nPadsSignal-1,PadNumber+3);

     Int_t pmin=LeftPad-PadNumber+3; // lower index of the pad_signal vector
     Int_t pmax=RightPad-PadNumber+3; // upper index  


     Float_t z_drift = z*zwidthm1;
     Float_t z_offset = z_drift-(Int_t)z_drift;

     Int_t FirstBucket = (Int_t)z_drift; // numbering starts from "0"

     for (Int_t i2=0;i2<4;i2++){     
       Int_t TrueTime = FirstBucket+i2; // current time bucket
       Float_t dz   = (Float_t(i2)+1.- z_offset)*zwidth; 
       Float_t ampl = fRF->GetRF(dz); 
       if((TrueTime>MAXTBKT-1)) break; // beyond the time range


       // loop over pads, from pmin to pmax

       for(Int_t i3=pmin;i3<pmax+1;i3++){
         Int_t TruePad = LeftPad+i3-pmin;

         if(TruePad<nPadsDiff || TruePad > nPadsSignal-nPadsDiff-1) continue;

         TruePad -= nPadsDiff;
         signal(TruePad,TrueTime)+=(PadSignal[i3]*ampl); // not converted to charge!

       } // end of loop over pads
     } // end of loop over time bins

   } // end of loop over electrons 

 } // end of loop over tracks

} // end of GetCrossTalk



//_____________________________________________________________________________
void AliTPC::Init()
{
  //
  // Initialise TPC detector after definition of geometry
  //
  Int_t i;
  //
  printf("\n");
  for(i=0;i<35;i++) printf("*");
  printf(" TPC_INIT ");
  for(i=0;i<35;i++) printf("*");
  printf("\n");
  //
  for(i=0;i<80;i++) printf("*");
  printf("\n");
}

//_____________________________________________________________________________
void AliTPC::MakeBranch(Option_t* option)
{
  //
  // Create Tree branches for the TPC.
  //
  Int_t buffersize = 4000;
  char branchname[10];
  sprintf(branchname,"%s",GetName());

  AliDetector::MakeBranch(option);

  char *D = strstr(option,"D");

  if (fDigits   && gAlice->TreeD() && D) {
    gAlice->TreeD()->Branch(branchname,&fDigits, buffersize);
    printf("Making Branch %s for digits\n",branchname);
  }	

  char *R = strstr(option,"R");

  if (fClusters && gAlice->TreeR() && R) {
    gAlice->TreeR()->Branch(branchname,&fClusters, buffersize);
    printf("Making Branch %s for Clusters\n",branchname);
  }	
}
 
//_____________________________________________________________________________
void AliTPC::ResetDigits()
{
  //
  // Reset number of digits and the digits array for this detector
  // reset clusters
  //
  fNdigits   = 0;
  //  if (fDigits)   fDigits->Clear();
  if (fDigParam->GetArray()!=0)  fDigParam->GetArray()->Clear();
  fNclusters = 0;
  if (fClusters) fClusters->Clear();
}

//_____________________________________________________________________________
void AliTPC::SetSecAL(Int_t sec)
{
  //---------------------------------------------------
  // Activate/deactivate selection for lower sectors
  //---------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  fSecAL = sec;
}

//_____________________________________________________________________________
void AliTPC::SetSecAU(Int_t sec)
{
  //----------------------------------------------------
  // Activate/deactivate selection for upper sectors
  //---------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  fSecAU = sec;
}

//_____________________________________________________________________________
void AliTPC::SetSecLows(Int_t s1,Int_t s2,Int_t s3,Int_t s4,Int_t s5, Int_t s6)
{
  //----------------------------------------
  // Select active lower sectors
  //----------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  fSecLows[0] = s1;
  fSecLows[1] = s2;
  fSecLows[2] = s3;
  fSecLows[3] = s4;
  fSecLows[4] = s5;
  fSecLows[5] = s6;
}

//_____________________________________________________________________________
void AliTPC::SetSecUps(Int_t s1,Int_t s2,Int_t s3,Int_t s4,Int_t s5, Int_t s6,
                       Int_t s7, Int_t s8 ,Int_t s9 ,Int_t s10, 
                       Int_t s11 , Int_t s12)
{
  //--------------------------------
  // Select active upper sectors
  //--------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  fSecUps[0] = s1;
  fSecUps[1] = s2;
  fSecUps[2] = s3;
  fSecUps[3] = s4;
  fSecUps[4] = s5;
  fSecUps[5] = s6;
  fSecUps[6] = s7;
  fSecUps[7] = s8;
  fSecUps[8] = s9;
  fSecUps[9] = s10;
  fSecUps[10] = s11;
  fSecUps[11] = s12;
}

//_____________________________________________________________________________
void AliTPC::SetSens(Int_t sens)
{

  //-------------------------------------------------------------
  // Activates/deactivates the sensitive strips at the center of
  // the pad row -- this is for the space-point resolution calculations
  //-------------------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  fSens = sens;
}
 
void AliTPC::SetSide(Float_t side)
{
  fSide = side;
 
}
//____________________________________________________________________________
void AliTPC::SetGasMixt(Int_t nc,Int_t c1,Int_t c2,Int_t c3,Float_t p1,
                           Float_t p2,Float_t p3)
{

 fNoComp = nc;
 
 fMixtComp[0]=c1;
 fMixtComp[1]=c2;
 fMixtComp[2]=c3;

 fMixtProp[0]=p1;
 fMixtProp[1]=p2;
 fMixtProp[2]=p3; 
 
 
}
//_____________________________________________________________________________
void AliTPC::Streamer(TBuffer &R__b)
{
  //
  // Stream an object of class AliTPC.
  //
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(); if (R__v) { }
      AliDetector::Streamer(R__b);
      if (R__v < 2) return;
      R__b >> fNsectors;
      R__b >> fNclusters;
      R__b >> fNtracks;
      fClustersIndex = new Int_t[fNsectors+1];
      fDigitsIndex   = new Int_t[fNsectors+1];
   } else {
      R__b.WriteVersion(AliTPC::IsA());
      AliDetector::Streamer(R__b);
      R__b << fNsectors;
      R__b << fNclusters;
      R__b << fNtracks;
   }
}
  
ClassImp(AliTPCcluster)
 
//_____________________________________________________________________________
AliTPCcluster::AliTPCcluster(Float_t *hits, Int_t *lab)
{
  //
  // Creates a simulated cluster for the TPC
  //
  fTracks[0]  = lab[0];
  fTracks[1]  = lab[1];
  fTracks[2]  = lab[2];
  fSector     = lab[3];
  fPadRow     = lab[4];
  fY          = hits[0];
  fZ          = hits[1];
  fQ          = hits[2];
  fSigmaY2    = hits[3];
  fSigmaZ2    = hits[4];
}
 
//_____________________________________________________________________________
void AliTPCcluster::GetXYZ(Float_t *x, const AliTPCParam *par) const 
{
  //
  // Transformation from local to global coordinate system
  //
  x[0]=par->GetPadRowRadii(fSector,fPadRow);
  x[1]=fY;
  x[2]=fZ;
  par->CRXYZtoXYZ(x,fSector,fPadRow,1);
  x[2]=fZ;
}
 
//_____________________________________________________________________________
Int_t AliTPCcluster::Compare(TObject * o)
{
  //
  // compare two clusters according y coordinata
  //
  AliTPCcluster *cl= (AliTPCcluster *)o;
  if (fY<cl->fY) return -1;
  if (fY==cl->fY) return 0;
  return 1;  
}

Bool_t AliTPCcluster::IsSortable() const
{
  //
  //make AliTPCcluster sortabale
  //
  return kTRUE; 
}



ClassImp(AliTPCdigit)
 
//_____________________________________________________________________________
AliTPCdigit::AliTPCdigit(Int_t *tracks, Int_t *digits):
  AliDigit(tracks)
{
  //
  // Creates a TPC digit object
  //
  fSector     = digits[0];
  fPadRow     = digits[1];
  fPad        = digits[2];
  fTime       = digits[3];
  fSignal     = digits[4];
}

 
ClassImp(AliTPChit)
 
//_____________________________________________________________________________
AliTPChit::AliTPChit(Int_t shunt, Int_t track, Int_t *vol, Float_t *hits):
AliHit(shunt,track)
{
  //
  // Creates a TPC hit object
  //
  fSector     = vol[0];
  fPadRow     = vol[1];
  fX          = hits[0];
  fY          = hits[1];
  fZ          = hits[2];
  fQ          = hits[3];
}
 
 
ClassImp(AliTPCtrack)
 
//_____________________________________________________________________________
AliTPCtrack::AliTPCtrack(Float_t *hits)
{
  //
  // Default creator for a TPC reconstructed track object
  //
  fX=hits[0]; // This is dummy code !
}

AliTPCtrack::AliTPCtrack(const AliTPCcluster *c,const TVector& xx,
			 const TMatrix& CC, Double_t xref, Double_t alpha):
  x(xx),C(CC),fClusters(200)
{
  //-----------------------------------------------------------------
  // This is the main track constructor.
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  fX=xref;
  fAlpha=alpha;
  fChi2=0.;
  fClusters.AddLast((AliTPCcluster*)(c));
}

//_____________________________________________________________________________
AliTPCtrack::AliTPCtrack(const AliTPCtrack& t) : x(t.x), C(t.C),
  fClusters(t.fClusters.GetEntriesFast()) 
{
  //-----------------------------------------------------------------
  // This is a track copy constructor.
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  fX=t.fX;
  fChi2=t.fChi2;
  fAlpha=t.fAlpha;
  int n=t.fClusters.GetEntriesFast();
  for (int i=0; i<n; i++) fClusters.AddLast(t.fClusters.UncheckedAt(i));
}

//_____________________________________________________________________________
Int_t AliTPCtrack::Compare(TObject *o) {
  //-----------------------------------------------------------------
  // This function compares tracks according to their curvature.
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  AliTPCtrack *t=(AliTPCtrack*)o;
  Double_t co=t->GetSigmaY2();
  Double_t c =GetSigmaY2();
  if (c>co) return 1;
  else if (c<co) return -1;
  return 0;
}

//_____________________________________________________________________________
int AliTPCtrack::PropagateTo(Double_t xk,Double_t x0,Double_t rho,Double_t pm)
{
  //-----------------------------------------------------------------
  // This function propagates a track to a reference plane x=xk.
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  if (TMath::Abs(x(2)*xk - x(3)) >= 0.999) {
    if (*this>4) cerr<<*this<<" AliTPCtrack warning: Propagation failed !\n";
    return 0;
  }

  Double_t x1=fX, x2=x1+0.5*(xk-x1), dx=x2-x1, y1=x(0), z1=x(1);
  Double_t c1=x(2)*x1 - x(3), r1=sqrt(1.- c1*c1);
  Double_t c2=x(2)*x2 - x(3), r2=sqrt(1.- c2*c2);
  
  x(0) += dx*(c1+c2)/(r1+r2);
  x(1) += dx*(c1+c2)/(c1*r2 + c2*r1)*x(4);

  TMatrix F(5,5); F.UnitMatrix();
  Double_t rr=r1+r2, cc=c1+c2, xx=x1+x2;
  F(0,2)= dx*(rr*xx + cc*(c1*x1/r1+c2*x2/r2))/(rr*rr);
  F(0,3)=-dx*(2*rr + cc*(c1/r1 + c2/r2))/(rr*rr);
  Double_t cr=c1*r2+c2*r1;
  F(1,2)= dx*x(4)*(cr*xx-cc*(r1*x2-c2*c1*x1/r1+r2*x1-c1*c2*x2/r2))/(cr*cr);
  F(1,3)=-dx*x(4)*(2*cr + cc*(c2*c1/r1-r1 + c1*c2/r2-r2))/(cr*cr);
  F(1,4)= dx*cc/cr; 
  TMatrix tmp(F,TMatrix::kMult,C);
  C.Mult(tmp,TMatrix(TMatrix::kTransposed,F));
  
  fX=x2;
  
  //Multiple scattering******************
  Double_t ey=x(2)*fX - x(3);
  Double_t ex=sqrt(1-ey*ey);
  Double_t ez=x(4);
  TMatrix Q(5,5); Q=0.;
  Q(2,2)=ez*ez+ey*ey;   Q(2,3)=-ex*ey;       Q(2,4)=-ex*ez;
  Q(3,2)=Q(2,3);        Q(3,3)= ez*ez+ex*ex; Q(3,4)=-ey*ez;
  Q(4,2)=Q(2,4);        Q(4,3)= Q(3,4);      Q(4,4)=1.;
  
  F=0;
  F(2,2)=-x(2)*ex;          F(2,3)=-x(2)*ey;
  F(3,2)=-ex*(x(2)*fX-ey);  F(3,3)=-(1.+ x(2)*fX*ey - ey*ey);
  F(4,2)=-ez*ex;            F(4,3)=-ez*ey;           F(4,4)=1.;
  
  tmp.Mult(F,Q);
  Q.Mult(tmp,TMatrix(TMatrix::kTransposed,F));
  
  Double_t p2=GetPt()*GetPt()*(1.+x(4)*x(4));
  Double_t beta2=p2/(p2 + pm*pm);
  Double_t d=sqrt((x1-fX)*(x1-fX)+(y1-x(0))*(y1-x(0))+(z1-x(1))*(z1-x(1)));
  d*=2.;
  Double_t theta2=14.1*14.1/(beta2*p2*1e6)*d/x0*rho;
  Q*=theta2;
  C+=Q;
  
  //Energy losses************************
  Double_t dE=0.153e-3/beta2*(log(5940*beta2/(1-beta2)) - beta2)*d*rho;
  if (x1 < x2) dE=-dE;
  x(2)*=(1.- sqrt(p2+pm*pm)/p2*dE);
  //x(3)*=(1.- sqrt(p2+pm*pm)/p2*dE);
  
  x1=fX; x2=xk; y1=x(0); z1=x(1);
  c1=x(2)*x1 - x(3); r1=sqrt(1.- c1*c1);
  c2=x(2)*x2 - x(3); r2=sqrt(1.- c2*c2);
  
  x(0) += dx*(c1+c2)/(r1+r2);
  x(1) += dx*(c1+c2)/(c1*r2 + c2*r1)*x(4);
  
  F.UnitMatrix();
  rr=r1+r2; cc=c1+c2; xx=x1+x2;
  F(0,2)= dx*(rr*xx + cc*(c1*x1/r1+c2*x2/r2))/(rr*rr);
  F(0,3)=-dx*(2*rr + cc*(c1/r1 + c2/r2))/(rr*rr);
  cr=c1*r2+c2*r1;
  F(1,2)= dx*x(4)*(cr*xx-cc*(r1*x2-c2*c1*x1/r1+r2*x1-c1*c2*x2/r2))/(cr*cr);
  F(1,3)=-dx*x(4)*(2*cr + cc*(c2*c1/r1-r1 + c1*c2/r2-r2))/(cr*cr);
  F(1,4)= dx*cc/cr; 
  tmp.Mult(F,C);
  C.Mult(tmp,TMatrix(TMatrix::kTransposed,F));
  
  fX=x2;
  
  return 1;
}

//_____________________________________________________________________________
void AliTPCtrack::PropagateToVertex(Double_t x0,Double_t rho,Double_t pm) 
{
  //-----------------------------------------------------------------
  // This function propagates tracks to the "vertex".
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  Double_t c=x(2)*fX - x(3);
  Double_t tgf=-x(3)/(x(2)*x(0) + sqrt(1-c*c));
  Double_t snf=tgf/sqrt(1.+ tgf*tgf);
  Double_t xv=(x(3)+snf)/x(2);
  PropagateTo(xv,x0,rho,pm);
}

//_____________________________________________________________________________
void AliTPCtrack::Update(const AliTPCcluster *c, Double_t chisq)
{
  //-----------------------------------------------------------------
  // This function associates a clusters with this track.
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  TMatrix H(2,5); H.UnitMatrix();
  TMatrix Ht(TMatrix::kTransposed,H);
  TVector m(2);   m(0)=c->fY; m(1)=c->fZ; 
  TMatrix V(2,2); V(0,0)=c->fSigmaY2; V(0,1)=0.; V(1,0)=0.; V(1,1)=c->fSigmaZ2;

  TMatrix tmp(H,TMatrix::kMult,C);
  TMatrix R(tmp,TMatrix::kMult,Ht); R+=V;
  
  Double_t det=(Double_t)R(0,0)*R(1,1) - (Double_t)R(0,1)*R(1,0);
  R(0,1)=R(0,0); R(0,0)=R(1,1); R(1,1)=R(0,1); 
  R(1,0)*=-1; R(0,1)=R(1,0);
  R*=1./det;
  
  //R.Invert();
  
  TMatrix K(C,TMatrix::kMult,Ht); K*=R;
  
  TVector savex=x;
  x*=H; x-=m; x*=-1; x*=K; x+=savex;
  if (TMath::Abs(x(2)*fX-x(3)) >= 0.999) {
    if (*this>4) cerr<<*this<<" AliTPCtrack warning: Filtering failed !\n";
    x=savex;
    return;
  }
  
  TMatrix saveC=C;
  C.Mult(K,tmp); C-=saveC; C*=-1;
  
  fClusters.AddLast((AliTPCcluster*)c);
  fChi2 += chisq;
}

//_____________________________________________________________________________
int AliTPCtrack::Rotate(Double_t alpha)
{
  //-----------------------------------------------------------------
  // This function rotates this track.
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  fAlpha += alpha;
  
  Double_t x1=fX, y1=x(0);
  Double_t ca=cos(alpha), sa=sin(alpha);
  Double_t r1=x(2)*fX - x(3);
  
  fX = x1*ca + y1*sa;
  x(0)=-x1*sa + y1*ca;
  x(3)=x(3)*ca + (x(2)*y1 + sqrt(1.- r1*r1))*sa;
  
  Double_t r2=x(2)*fX - x(3);
  if (TMath::Abs(r2) >= 0.999) {
    if (*this>4) cerr<<*this<<" AliTPCtrack warning: Rotation failed !\n";
    return 0;
  }
  
  Double_t y0=x(0) + sqrt(1.- r2*r2)/x(2);
  if ((x(0)-y0)*x(2) >= 0.) {
    if (*this>4) cerr<<*this<<" AliTPCtrack warning: Rotation failed !!!\n";
    return 0;
  }
  
  TMatrix F(5,5); F.UnitMatrix();
  F(0,0)=ca;
  F(3,0)=x(2)*sa; 
  F(3,2)=(y1 - r1*x1/sqrt(1.- r1*r1))*sa; 
  F(3,3)= ca + sa*r1/sqrt(1.- r1*r1);
  TMatrix tmp(F,TMatrix::kMult,C); 
  // Double_t dy2=C(0,0);
  C.Mult(tmp,TMatrix(TMatrix::kTransposed,F));
  // C(0,0)+=dy2*sa*sa*r1*r1/(1.- r1*r1);
  // C(1,1)+=dy2*sa*sa*x(4)*x(4)/(1.- r1*r1);
  
  return 1;
}

//_____________________________________________________________________________
void AliTPCtrack::UseClusters() const 
{
  //-----------------------------------------------------------------
  // This function marks clusters associated with this track.
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  int num_of_clusters=fClusters.GetEntriesFast();
  for (int i=0; i<num_of_clusters; i++) {
    //if (i<=14) continue;
    AliTPCcluster *c=(AliTPCcluster*)fClusters.UncheckedAt(i);
    c->Use();   
  }
}

//_____________________________________________________________________________
Double_t AliTPCtrack::GetPredictedChi2(const AliTPCcluster *c) const 
{
  //-----------------------------------------------------------------
  // This function calculates a predicted chi2 increment.
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  TMatrix H(2,5); H.UnitMatrix();
  TVector m(2);   m(0)=c->fY; m(1)=c->fZ; 
  TMatrix V(2,2); V(0,0)=c->fSigmaY2; V(0,1)=0.; V(1,0)=0.; V(1,1)=c->fSigmaZ2;
  TVector res=x;  res*=H; res-=m; //res*=-1; 
  TMatrix tmp(H,TMatrix::kMult,C);
  TMatrix R(tmp,TMatrix::kMult,TMatrix(TMatrix::kTransposed,H)); R+=V;
  
  Double_t det=(Double_t)R(0,0)*R(1,1) - (Double_t)R(0,1)*R(1,0);
  if (TMath::Abs(det) < 1.e-10) {
    if (*this>4) cerr<<*this<<" AliTPCtrack warning: Singular matrix !\n";
    return 1e10;
  }
  R(0,1)=R(0,0); R(0,0)=R(1,1); R(1,1)=R(0,1); 
  R(1,0)*=-1; R(0,1)=R(1,0);
  R*=1./det;
  
  //R.Invert();
  
  TVector r=res;
  res*=R;
  return r*res;
}

//_____________________________________________________________________________
struct S { int lab; int max; };
int AliTPCtrack::GetLabel(int nrows) const 
{
  //-----------------------------------------------------------------
  // This function returns the track label. If label<0, this track is fake.
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  int num_of_clusters=fClusters.GetEntriesFast();
  S *s=new S[num_of_clusters];
  int i;
  for (i=0; i<num_of_clusters; i++) s[i].lab=s[i].max=0;
  
  int lab=123456789;
  for (i=0; i<num_of_clusters; i++) {
    AliTPCcluster *c=(AliTPCcluster*)fClusters.UncheckedAt(i);
    lab=TMath::Abs(c->fTracks[0]);
    int j;
    for (j=0; j<num_of_clusters; j++)
      if (s[j].lab==lab || s[j].max==0) break;
    s[j].lab=lab;
    s[j].max++;
  }
  
  int max=0;
  for (i=0; i<num_of_clusters; i++) 
    if (s[i].max>max) {max=s[i].max; lab=s[i].lab;}
    
  delete[] s;
  
  for (i=0; i<num_of_clusters; i++) {
    AliTPCcluster *c=(AliTPCcluster*)fClusters.UncheckedAt(i);
    if (TMath::Abs(c->fTracks[1]) == lab ||
        TMath::Abs(c->fTracks[2]) == lab ) max++;
  }
  
  if (1.-float(max)/num_of_clusters > 0.10) return -lab;
  
  int tail=int(0.08*nrows);
  if (num_of_clusters < tail) return lab;
  
  max=0;
  for (i=1; i<=tail; i++) {
    AliTPCcluster *c=(AliTPCcluster*)fClusters.UncheckedAt(num_of_clusters-i);
    if (lab == TMath::Abs(c->fTracks[0]) ||
        lab == TMath::Abs(c->fTracks[1]) ||
        lab == TMath::Abs(c->fTracks[2])) max++;
  }
  if (max < int(0.5*tail)) return -lab;
  
  return lab;
}

//_____________________________________________________________________________
void AliTPCtrack::GetPxPyPz(Double_t& px, Double_t& py, Double_t& pz) const 
{
  //-----------------------------------------------------------------
  // This function returns reconstructed track momentum in the global system.
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  Double_t pt=TMath::Abs(GetPt()); // GeV/c
  Double_t r=x(2)*fX-x(3);
  Double_t y0=x(0) + sqrt(1.- r*r)/x(2);
  px=-pt*(x(0)-y0)*x(2);    //cos(phi);
  py=-pt*(x(3)-fX*x(2));   //sin(phi);
  pz=pt*x(4);
  Double_t tmp=px*TMath::Cos(fAlpha) - py*TMath::Sin(fAlpha);
  py=px*TMath::Sin(fAlpha) + py*TMath::Cos(fAlpha);
  px=tmp;  
}

//_____________________________________________________________________________
Double_t AliTPCtrack::GetdEdX(Double_t low, Double_t up) const {
  //-----------------------------------------------------------------
  // This funtion calculates dE/dX within the "low" and "up" cuts.
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------
  int ncl=fClusters.GetEntriesFast();
  int n=0;
  Double_t *q=new Double_t[ncl];
  int i;
  for (i=0; i<ncl; i++) {
     AliTPCcluster *cl=(AliTPCcluster*)(fClusters.UncheckedAt(i));
     //     if (cl->fdEdX > 3000) continue;
     if (cl->fdEdX <= 0) continue;
     q[n++]=cl->fdEdX;
  }

  //stupid sorting
  int swap;
  do {
    swap=0;
    for (i=0; i<n-1; i++) {
      if (q[i]<=q[i+1]) continue;
      Double_t tmp=q[i]; q[i]=q[i+1]; q[i+1]=tmp;
      swap++;
    }
  } while (swap);

  int nl=int(low*n), nu=int(up *n);
  Double_t dedx=0.;
  for (i=nl; i<=nu; i++) dedx += q[i];
  dedx /= (nu-nl+1);
  return dedx;
}

//_________________________________________________________________________
//
// Classes for internal tracking use
//_________________________________________________________________________
void AliTPCRow::InsertCluster(const AliTPCcluster* c) {
  //-----------------------------------------------------------------------
  // Insert a cluster into this pad row in accordence with its y-coordinate
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------------
  if (num_of_clusters==MAX_CLUSTER_PER_ROW) {
    cerr<<"AliTPCRow::InsertCluster(): Too many clusters !\n"; return;
  }
  if (num_of_clusters==0) {clusters[num_of_clusters++]=c; return;}
  int i=Find(c->fY);
  memmove(clusters+i+1 ,clusters+i,(num_of_clusters-i)*sizeof(AliTPCcluster*));
  clusters[i]=c; num_of_clusters++;
}

int AliTPCRow::Find(Double_t y) const {
  //-----------------------------------------------------------------------
  // Return the index of the nearest cluster 
  //
  // Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
  //-----------------------------------------------------------------------
  if (y <= clusters[0]->fY) return 0;
  if (y > clusters[num_of_clusters-1]->fY) return num_of_clusters;
  int b=0, e=num_of_clusters-1, m=(b+e)/2;
  for (; b<e; m=(b+e)/2) {
    if (y > clusters[m]->fY) b=m+1;
    else e=m; 
  }
  return m;
}

