#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TAxis.h"
#include "TPad.h"
#include "TFitResult.h"
#include <iostream>
#include <iomanip>

const double FIT_MIN_SI = 0.0;
const double FIT_MAX_SI = 1000.0;
const double FIT_MIN_GE = 0.0;
const double FIT_MAX_GE = 180.0;

// Helper: Standardize graph appearance
void ApplyStyle(TGraphErrors* gr, int color, const char* title) {
    gr->SetMarkerStyle(20);
    gr->SetMarkerSize(1.0);
    gr->SetMarkerColor(color);
    gr->SetLineColor(color);
    gr->SetTitle(title);
    gr->GetXaxis()->SetTitleSize(0.045);
    gr->GetYaxis()->SetTitleSize(0.045);
}

// Helper: Standardize fit function
TF1* CreateFitFunc(const char* name, double min, double max, double p0, double p1) {
    TF1* f = new TF1(name, "[0]*(exp(x/[1])-1)", min, max);
    f->SetParNames("I_{0}", "#eta V_{t}");
    f->SetParameters(p0, p1); // Initial guesses
    f->SetLineColor(kBlack);
    f->SetLineWidth(2);
    return f;
}

void figure() {
    // Canvas Setup
    gStyle->SetOptFit(1111);
    gStyle->SetOptStat(0);
    
    TCanvas *c1 = new TCanvas("c_sidebyside", "Diodi Side-by-Side", 1200, 600);
    c1->Divide(2, 1);

    // --- DATA LOADING ---
    TGraphErrors *gr_si = new TGraphErrors("data/dati_silicio.txt", "%lg %lg %lg %lg");
    TGraphErrors *gr_ge = new TGraphErrors("data/dati_germanio.txt", "%lg %lg %lg %lg");

    if (gr_si->GetN() == 0 || gr_ge->GetN() == 0) {
        std::cout << "Error: Data file empty or not found." << std::endl;
        return;
    }

    // --- STYLING ---
    ApplyStyle(gr_si, kBlue, "Silicon Diode;Voltage (mV);Current (mA)");
    ApplyStyle(gr_ge, kRed,  "Germanium Diode;Voltage (mV);Current (mA)");

    // --- FITTING ---
    TF1 *f_si = CreateFitFunc("f_si", FIT_MIN_SI, FIT_MAX_SI, 1e-6, 40.0);
    TF1 *f_ge = CreateFitFunc("f_ge", FIT_MIN_GE, FIT_MAX_GE, 0.005, 30.0);

    // Perform fits (Q = Quiet, S = Store results)
    TFitResultPtr r_si = gr_si->Fit(f_si, "QSR");
    TFitResultPtr r_ge = gr_ge->Fit(f_ge, "QSR");

    // --- DRAWING ---
    
    // Pad 1: Silicon
    c1->cd(1);
    gPad->SetGrid();
    gPad->SetLogy();
    gPad->SetLeftMargin(0.12); // Adjust margins for axis labels
    gr_si->Draw("AP");

    TLegend *leg1 = new TLegend(0.15, 0.75, 0.55, 0.88);
    leg1->AddEntry(gr_si, "Data (Si)", "p");
    leg1->AddEntry(f_si, "Fit: I_{0}(e^{V/#eta V_{T}}-1)", "l");
    leg1->Draw();

    // Pad 2: Germanium
    c1->cd(2);
    gPad->SetGrid();
    gPad->SetLogy();
    gPad->SetLeftMargin(0.12);
    gr_ge->Draw("AP");

    TLegend *leg2 = new TLegend(0.15, 0.75, 0.55, 0.88);
    leg2->AddEntry(gr_ge, "Data (Ge)", "p");
    leg2->AddEntry(f_ge, "Fit: I_{0}(e^{V/#eta V_{T}}-1)", "l");
    leg2->Draw();

    // --- OUTPUT ---
    auto print_res = [](const char* label, TF1* f, TFitResultPtr& r) {
        std::cout << "\n>>> " << label << " RESULTS:\n"
                  << "-----------------------------------\n";
        if (r.Get()) { // Ensure fit succeeded
            std::cout << "  I0     = " << f->GetParameter(0) << " +/- " << f->GetParError(0) << " mA\n"
                      << "  eta*Vt = " << f->GetParameter(1) << " +/- " << f->GetParError(1) << " mV\n"
                      << "  Chi2/NDF = " << f->GetChisquare() << "/" << f->GetNDF() 
                      << " (Prob: " << f->GetProb() << ")\n";
        } else {
            std::cout << "  FIT FAILED\n";
        }
        std::cout << "-----------------------------------\n";
    };

    std::cout << std::fixed << std::setprecision(6);
    print_res("SILICON", f_si, r_si);
    print_res("GERMANIUM", f_ge, r_ge);
}