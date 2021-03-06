#include <memory>
#include <iostream>

#include "Geometry/Records/interface/IdealGeometryRecord.h"
#include "Geometry/Records/interface/HcalSimNumberingRecord.h"
#include "Geometry/HGCalCommonData/interface/HGCalDDDConstants.h"
#include "Geometry/HcalCommonData/interface/HcalDDDSimConstants.h"
#include "Geometry/HcalCommonData/interface/HcalCellType.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/ForwardDetId/interface/ForwardSubdetector.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/Core/interface/DQMStore.h"

#include "SimG4CMS/Calo/interface/HGCNumberingScheme.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "SimDataFormats/CaloHit/interface/PCaloHitContainer.h"
#include "SimDataFormats/ValidationFormats/interface/PHGCalValidInfo.h"
#include "DataFormats/HcalDetId/interface/HcalTestNumbering.h"
#include "SimDataFormats/CaloTest/interface/HGCalTestNumbering.h"

#include "PhysicsTools/HepMCCandAlgos/interface/GenParticlesHelper.h"

#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Units/GlobalSystemOfUnits.h"
#include "CLHEP/Units/GlobalPhysicalConstants.h"

class HGCGeometryValidation : public DQMEDAnalyzer {

public:

  explicit HGCGeometryValidation( const edm::ParameterSet& );
  ~HGCGeometryValidation() override;
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

protected:
  void dqmBeginRun(edm::Run const&, edm::EventSetup const&) override;
  void bookHistograms(DQMStore::IBooker &, edm::Run const &, edm::EventSetup const &) override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;

private:
  edm::EDGetTokenT<PHGCalValidInfo> g4Token_;
  std::vector<std::string> geometrySource_;

  //HGCal geometry scheme
  std::vector<const HGCalDDDConstants*> hgcGeometry_;
  const HcalDDDSimConstants *hcons_;

  //histogram related stuff
  MonitorElement *heedzVsZ, *heedyVsY, *heedxVsX;
  MonitorElement *hefdzVsZ, *hefdyVsY, *hefdxVsX;
  MonitorElement *hebdzVsZ, *hebdyVsY, *hebdxVsX;
  MonitorElement *heedzVsLayer, *hefdzVsLayer, *hebdzVsLayer;
  MonitorElement *heedyVsLayer, *hefdyVsLayer, *hebdyVsLayer;
  MonitorElement *heedxVsLayer, *hefdxVsLayer, *hebdxVsLayer;
  MonitorElement *heeXG4VsId,   *hefXG4VsId,   *hebXG4VsId;
  MonitorElement *heeYG4VsId,   *hefYG4VsId,   *hebYG4VsId;
  MonitorElement *heeZG4VsId,   *hefZG4VsId,   *hebZG4VsId;
  MonitorElement *hebLayerVsEnStep, *hefLayerVsEnStep, *heeLayerVsEnStep;
	
  MonitorElement *heeTotEdepStep, *hefTotEdepStep, *hebTotEdepStep;
  MonitorElement *heedX, *heedY, *heedZ;
  MonitorElement *hefdX, *hefdY, *hefdZ;
  MonitorElement *hebdX, *hebdY, *hebdZ;
};

HGCGeometryValidation::HGCGeometryValidation(const edm::ParameterSet &cfg) : hcons_(nullptr) {

  g4Token_ = consumes<PHGCalValidInfo>(cfg.getParameter<edm::InputTag>("g4Source"));
  geometrySource_ = cfg.getUntrackedParameter< std::vector<std::string> >("geometrySource");
}

HGCGeometryValidation::~HGCGeometryValidation() { }

void HGCGeometryValidation::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

void HGCGeometryValidation::dqmBeginRun(const edm::Run& iRun, 
					const edm::EventSetup& iSetup) {

  //initiating hgcnumbering
  for (size_t i=0; i<geometrySource_.size(); i++) {
    if (geometrySource_[i].find("Hcal") != std::string::npos) {
      edm::ESHandle<HcalDDDSimConstants> pHRNDC;
      iSetup.get<HcalSimNumberingRecord>().get(pHRNDC);
      if (pHRNDC.isValid()) {
	hcons_ = &(*pHRNDC);
	hgcGeometry_.push_back(nullptr);
      } else {
	edm::LogWarning("HGCalValid") << "Cannot initiate HGCalGeometry for "
				      << geometrySource_[i] << std::endl;
      }
    } else {
      edm::ESHandle<HGCalDDDConstants> hgcGeom;
      iSetup.get<IdealGeometryRecord>().get(geometrySource_[i],hgcGeom);
      if (hgcGeom.isValid()) {
	hgcGeometry_.push_back(hgcGeom.product());
      } else {
	edm::LogWarning("HGCalValid") << "Cannot initiate HGCalGeometry for "
				      << geometrySource_[i] << std::endl;
      }
    }
  }
}


void HGCGeometryValidation::bookHistograms(DQMStore::IBooker& iB, 
                                           edm::Run const&, 
                                           edm::EventSetup const&) {

  iB.setCurrentFolder("HGCalSimHitsV/Geometry");

  //initiating histograms
  heeTotEdepStep = iB.book1D("heeTotEdepStep","",100,0,100);
  hefTotEdepStep = iB.book1D("hefTotEdepStep","",100,0,100); 
  hebTotEdepStep = iB.book1D("hebTotEdepStep","",100,0,100);

  hebLayerVsEnStep = iB.book2D("hebLayerVsEnStep","",25,0,25,100,0,0.01);
  hefLayerVsEnStep = iB.book2D("hefLayerVsEnStep","",36,0,36,100,0,0.01);
  heeLayerVsEnStep = iB.book2D("heeLayerVsEnStep","",84,0,84,100,0,0.01);

  heeXG4VsId = iB.book2D("heeXG4VsId","",600,-300,300,600,-300,300);
  heeYG4VsId = iB.book2D("heeYG4VsId","",600,-300,300,600,-300,300);
  heeZG4VsId = iB.book2D("heeZG4VsId","",3000,320,350,3000,320,350);

  hefXG4VsId = iB.book2D("hefXG4VsId","",600,-300,300,600,-300,300);
  hefYG4VsId = iB.book2D("hefYG4VsId","",600,-300,300,600,-300,300);
  hefZG4VsId = iB.book2D("hefZG4VsId","",6000,350,410,6000,350,410);

  hebXG4VsId = iB.book2D("hebXG4VsId","",600,-300,300,600,-300,300);
  hebYG4VsId = iB.book2D("hebYG4VsId","",600,-300,300,600,-300,300);
  hebZG4VsId = iB.book2D("hebZG4VsId","",220,400,620,220,400,620);

  heedzVsZ = iB.book2D("heedzVsZ","",600,320,350,100,-1,1);
  heedyVsY = iB.book2D("heedyVsY","",400,-200,200,100,-1,1);
  heedxVsX = iB.book2D("heedxVsX","",400,-200,200,100,-1,1);

  hefdzVsZ = iB.book2D("hefdzVsZ","",1200,350,410,100,-1,1);
  hefdyVsY = iB.book2D("hefdyVsY","",400,-200,200,100,-1,1);
  hefdxVsX = iB.book2D("hefdxVsX","",400,-200,200,100,-1,1);

  hebdzVsZ = iB.book2D("hebdzVsZ","",220,400,620,100,-5,5);
  hebdyVsY = iB.book2D("hebdyVsY","",400,-200,200,100,-5,5);
  hebdxVsX = iB.book2D("hebdxVsX","",400,-200,200,100,-5,5);

  heedzVsLayer = iB.book2D("heedzVsLayer","",100,0,100,100,-1,1);
  hefdzVsLayer = iB.book2D("hefdzVsLayer","",40,0,40,100,-1,1);
  hebdzVsLayer = iB.book2D("hebdzVsLayer","",50,0,25,100,-5,5);

  heedyVsLayer = iB.book2D("heedyVsLayer","",100,0,100,100,-1,1);
  hefdyVsLayer = iB.book2D("hefdyVsLayer","",40,0,40,100,-1,1);
  hebdyVsLayer = iB.book2D("hebdyVsLayer","",50,0,25,100,-5,5);

  heedxVsLayer = iB.book2D("heedxVsLayer","",100,0,100,100,-1,1);
  hefdxVsLayer = iB.book2D("hefdxVsLayer","",40,0,40,500,-1,1);
  hebdxVsLayer = iB.book2D("hebdxVsLayer","",50,0,25,500,-5,5.0);

  heedX = iB.book1D("heedX","",100,-1,1); 
  heedY = iB.book1D("heedY","",100,-1,1);
  heedZ = iB.book1D("heedZ","",100,-1,1);

  hefdX = iB.book1D("hefdX","",100,-1,1); 
  hefdY = iB.book1D("hefdY","",100,-1,1);
  hefdZ = iB.book1D("hefdZ","",100,-1,1);

  hebdX = iB.book1D("hebdX","",100,-1,1); 
  hebdY = iB.book1D("hebdY","",100,-1,1);
  hebdZ = iB.book1D("hebdZ","",100,-1,1);
}

void HGCGeometryValidation::analyze(const edm::Event &iEvent, 
				    const edm::EventSetup &iSetup) {

  //Accessing G4 information
  edm::Handle<PHGCalValidInfo> infoLayer;
  iEvent.getByToken(g4Token_,infoLayer);

  if (infoLayer.isValid()) {
    //step vertex information
    std::vector<float> hitVtxX = infoLayer->hitvtxX();
    std::vector<float> hitVtxY = infoLayer->hitvtxY();
    std::vector<float> hitVtxZ = infoLayer->hitvtxZ();
    std::vector<unsigned int> hitDet = infoLayer->hitDets();
    std::vector<unsigned int> hitIdx = infoLayer->hitIndex();

    //energy information
    std::vector<float> edepLayerEE = infoLayer->eehgcEdep();
    std::vector<float> edepLayerHE = infoLayer->hefhgcEdep();
    std::vector<float> edepLayerHB = infoLayer->hebhgcEdep();
    
    unsigned int i;		
    for(i=0; i<edepLayerEE.size(); i++) {
      heeLayerVsEnStep->Fill(i,edepLayerEE.at(i));
    }
	
    for(i=0; i<edepLayerHE.size(); i++)	{
      hefLayerVsEnStep->Fill(i,edepLayerHE.at(i));
    }

    for(i=0; i<edepLayerHB.size(); i++) {
      hebLayerVsEnStep->Fill(i,edepLayerHB.at(i));
    }

    //fill total energy deposited
    heeTotEdepStep->Fill((double)infoLayer->eeTotEdep());
    hefTotEdepStep->Fill((double)infoLayer->hefTotEdep()); 
    hebTotEdepStep->Fill((double)infoLayer->hebTotEdep());
    
    //loop over all hits
    for(unsigned int i=0; i<hitVtxX.size(); i++) {

      if (hitDet.at(i) == (unsigned int)(DetId::Forward)) {
	int subdet, zside, layer, wafer, celltype, cell;
	HGCalTestNumbering::unpackHexagonIndex(hitIdx.at(i), subdet, zside, layer, wafer, celltype, cell);	

	std::pair<float, float> xy;
	std::pair<int,float> layerIdx;
	double zp, xx, yx;

	if (subdet==(int)(HGCEE)) {
	  xy = hgcGeometry_[0]->locateCell(cell,layer,wafer,false); //mm
	  zp = hgcGeometry_[0]->waferZ(layer,false); //cm 
	  if (zside < 0) zp = -zp;
	  xx = (zp<0) ? -xy.first/10 : xy.first/10; //mm
	  yx = xy.second/10; //mm
	  hitVtxX.at(i) = hitVtxX.at(i)/10;
	  hitVtxY.at(i) = hitVtxY.at(i)/10;
	  hitVtxZ.at(i) = hitVtxZ.at(i)/10;
	  
	  heedzVsZ->Fill(zp, (hitVtxZ.at(i)-zp));
	  heedyVsY->Fill(yx, (hitVtxY.at(i)-yx));
	  heedxVsX->Fill(xx, (hitVtxX.at(i)-xx));
	  
	  heeXG4VsId->Fill(hitVtxX.at(i),xx);
	  heeYG4VsId->Fill(hitVtxY.at(i),yx);
	  heeZG4VsId->Fill(hitVtxZ.at(i),zp);

	  heedzVsLayer->Fill(layer,(hitVtxZ.at(i)-zp));
	  heedyVsLayer->Fill(layer,(hitVtxY.at(i)-yx));
	  heedxVsLayer->Fill(layer,(hitVtxX.at(i)-xx));
	  
	  heedX->Fill((hitVtxX.at(i)-xx));
	  heedZ->Fill((hitVtxZ.at(i)-zp));
	  heedY->Fill((hitVtxY.at(i)-yx));

	} else if (subdet==(int)(HGCHEF)) {

	  xy = hgcGeometry_[1]->locateCell(cell,layer,wafer,false); //mm
	  zp = hgcGeometry_[1]->waferZ(layer,false); //cm 
	  if (zside < 0) zp = -zp;
	  xx = (zp<0) ? -xy.first/10 : xy.first/10; //mm
	  yx = xy.second/10; //mm
	  hitVtxX.at(i) = hitVtxX.at(i)/10;
	  hitVtxY.at(i) = hitVtxY.at(i)/10;
	  hitVtxZ.at(i) = hitVtxZ.at(i)/10;
	  
	  hefdzVsZ->Fill(zp, (hitVtxZ.at(i)-zp));
	  hefdyVsY->Fill(yx, (hitVtxY.at(i)-yx));
	  hefdxVsX->Fill(xx, (hitVtxX.at(i)-xx));
	  
	  hefXG4VsId->Fill(hitVtxX.at(i),xx);
	  hefYG4VsId->Fill(hitVtxY.at(i),yx);
	  hefZG4VsId->Fill(hitVtxZ.at(i),zp);
	  
	  hefdzVsLayer->Fill(layer,(hitVtxZ.at(i)-zp));
	  hefdyVsLayer->Fill(layer,(hitVtxY.at(i)-yx));
	  hefdxVsLayer->Fill(layer,(hitVtxX.at(i)-xx));
	  
	  hefdX->Fill((hitVtxX.at(i)-xx));
	  hefdZ->Fill((hitVtxZ.at(i)-zp));
	  hefdY->Fill((hitVtxY.at(i)-yx));
	
	}
	
      } else if (hitDet.at(i) == (unsigned int)(DetId::Hcal)) {

	int subdet, zside, depth, eta, phi, lay;
	HcalTestNumbering::unpackHcalIndex(hitIdx.at(i), subdet, zside, depth, eta, phi, lay);
	HcalCellType::HcalCell cell = hcons_->cell(subdet, zside, lay, eta, phi);
	
	double zp = cell.rz/10; //mm --> cm
	if (zside == 0) zp = -zp;
	double rho = zp*tan(2.0*atan(exp(-cell.eta)));
	double xp  = rho * cos(cell.phi); //cm
	double yp  = rho * sin(cell.phi); //cm

	hitVtxX.at(i) = hitVtxX.at(i)/10;
	hitVtxY.at(i) = hitVtxY.at(i)/10;
	hitVtxZ.at(i) = hitVtxZ.at(i)/10;

	hebdzVsZ->Fill(zp, (hitVtxZ.at(i)-zp));
	hebdyVsY->Fill(yp, (hitVtxY.at(i)-yp));
	hebdxVsX->Fill(xp, (hitVtxX.at(i)-xp));

	hebXG4VsId->Fill(hitVtxX.at(i),xp);
	hebYG4VsId->Fill(hitVtxY.at(i),yp);
	hebZG4VsId->Fill(hitVtxZ.at(i),zp);

	hebdzVsLayer->Fill(lay,(hitVtxZ.at(i)-zp));
	hebdyVsLayer->Fill(lay,(hitVtxY.at(i)-yp));
	hebdxVsLayer->Fill(lay,(hitVtxX.at(i)-xp));

	hebdX->Fill((hitVtxX.at(i)-xp));
	hebdZ->Fill((hitVtxZ.at(i)-zp));
	hebdY->Fill((hitVtxY.at(i)-yp));
      }

    }//end G4 hits
    
  } else {
    edm::LogWarning("HGCalValid") << "No PHGCalInfo " << std::endl;
  }

}

//define this as a plug-in
DEFINE_FWK_MODULE(HGCGeometryValidation);






