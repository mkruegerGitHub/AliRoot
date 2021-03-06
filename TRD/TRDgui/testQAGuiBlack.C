
void testQAGuiBlack(const char *filename) {
  
  gROOT->SetStyle("Plain");
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);
  
  gSystem->Load("libGui");
  gSystem->Load("libTRDgui");
  
  AliTRDqaGuiMainBlack *fMain = new AliTRDqaGuiMainBlack(gClient->GetRoot());
  fMain->SetQAFile(filename);
}
