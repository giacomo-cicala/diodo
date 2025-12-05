#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TAxis.h"
#include "TPad.h"
#include "TFitResult.h"
#include "TPaveStats.h"
#include <iostream>
#include <iomanip>

const double FIT_MIN_SI = 0.0;
const double FIT_MAX_SI = 1000.0;
const double FIT_MIN_GE = 0.0;
const double FIT_MAX_GE = 180.0;

void fit_calibrazione()
{
    const char *filename = "data/dati_calibrazione.txt";

    TCanvas *c1 = new TCanvas("c1", "Calibrazione", 800, 600);
    c1->cd();
    gStyle->SetOptFit(1111);
    gStyle->SetOptStat(0);
    gPad->SetGrid();
    
    TGraphErrors *gr = new TGraphErrors(filename, "%lg %lg %lg %lg");

    if (gr->GetN() == 0)
    {
        std::cout << "Errore: " << filename << " vuoto o non trovato!" << std::endl;
        return;
    }

    gr->SetTitle("Calibrazione;Multimetro (mV);Oscilloscopio (mV)");
    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(kBlue);

    TF1 *f0 = new TF1("f_calib", "[0]+[1]*x", 0., 1000.);
    f0->SetLineColor(kRed);

    gr->Fit(f0, "Q");
    gr->Draw("AP");
    // Imposta range X e Y uguali e tick regolari da 0 a 900
    if (gPad) {
        gr->GetXaxis()->SetLimits(0, 900);
        gr->SetMinimum(0);
        gr->SetMaximum(900);
        gPad->Modified();
        gPad->Update();
        gr->GetXaxis()->SetNdivisions(9, kTRUE);
        gr->GetYaxis()->SetNdivisions(9, kTRUE);
    }

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n>>> Calibrazione (" << filename << " ):\n";
    std::cout << "Intercetta: " << f0->GetParameter(0) << " +/- " << f0->GetParError(0) << std::endl;
    std::cout << "Pendenza  : " << f0->GetParameter(1) << " +/- " << f0->GetParError(1) << std::endl;
}

// Helper: Standardize graph appearance
void ApplyStyle(TGraphErrors *gr, int color, const char *title)
{
    gr->SetMarkerStyle(20);
    gr->SetMarkerSize(1.0);
    gr->SetMarkerColor(color);
    gr->SetLineColor(color);
    gr->SetTitle(title);
}

// Helper: Standardize fit function
TF1 *CreateFitFunc(const char *name, double min, double max, double p0, double p1)
{
    TF1 *f = new TF1(name, "[0]*(exp(x/[1])-1)", min, max);
    f->SetParNames("I_{0}", "#eta V_{T}");
    f->SetParameters(p0, p1);
    f->SetLineColor(kBlack);
    return f;
}

void figure()
{
    // Canvas Setup
    gStyle->SetOptFit(1111);
    gStyle->SetOptStat(0);

    TCanvas *c1 = new TCanvas("caratteristica", "Diodi Side-by-Side", 1600, 600);
    c1->Divide(2, 1);

    // --- DATA LOADING ---
    TGraphErrors *gr_si = new TGraphErrors("data/dati_silicio.txt", "%lg %lg %lg %lg");
    TGraphErrors *gr_ge = new TGraphErrors("data/dati_germanio.txt", "%lg %lg %lg %lg");

    if (gr_si->GetN() == 0 || gr_ge->GetN() == 0)
    {
        std::cout << "Error: Data file empty or not found." << std::endl;
        return;
    }

    // --- STYLING ---
    ApplyStyle(gr_si, kBlue, "Diodo al Silicio;Tensione (mV);Corrente (mA)");
    ApplyStyle(gr_ge, kRed, "Diodo al Germanio;Tensione (mV);Corrente (mA)");

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
    gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.05);
    gr_si->GetYaxis()->SetNdivisions(10, kTRUE);
    gr_si->GetXaxis()->SetLimits(350, 700);
    gr_si->SetMinimum(1e-3); 
    gr_si->SetMaximum(3.0);
    gr_si->GetYaxis()->SetMoreLogLabels(kFALSE);
    gr_si->Draw("AP");

    gPad->Update();
    TPaveStats *st1 = (TPaveStats *)gr_si->FindObject("stats");
    if (st1)
    {
        st1->SetX1NDC(0.5); // Left edge
        st1->SetX2NDC(0.93); // Right edge
        st1->SetY1NDC(0.15); // Bottom edge
        st1->SetY2NDC(0.35); // Top edge
    }

    TLegend *leg1 = new TLegend(0.15, 0.70, 0.58, 0.88);
    leg1->AddEntry(gr_si, "Dati (Si)", "p");
    leg1->AddEntry(f_si, "Fit: I_{0}(e^{V/#eta V_{T}}-1)", "l");
    leg1->Draw();

    // Pad 2: Germanium
    c1->cd(2);
    gPad->SetGrid(1,1);
    gPad->SetLogy();
    gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.05);
    gr_ge->GetYaxis()->SetNdivisions(10, kTRUE);
    gr_ge->GetXaxis()->SetLimits(50, 400);
    gr_ge->SetMinimum(1e-3);
    gr_ge->SetMaximum(3.0);
    gr_ge->Draw("AP");

    gPad->Update();
    TPaveStats *st2 = (TPaveStats *)gr_ge->FindObject("stats");
    if (st2)
    {
        st2->SetX1NDC(0.5); // Left edge
        st2->SetX2NDC(0.93); // Right edge
        st2->SetY1NDC(0.15); // Bottom edge
        st2->SetY2NDC(0.35); // Top edge
    }

    TLegend *leg2 = new TLegend(0.15, 0.70, 0.58, 0.88);
    leg2->AddEntry(gr_ge, "Dati (Ge)", "p");
    leg2->AddEntry(f_ge, "Fit: I_{0}(e^{V/#eta V_{T}}-1)", "l");
    leg2->Draw();

    // --- OUTPUT ---
    auto print_res = [](const char *label, TF1 *f, TFitResultPtr &r)
    {
        std::cout << "\n>>> " << label << " RESULTS:\n"
                  << "-----------------------------------\n";
        if (r.Get())
        { // Ensure fit succeeded
            std::cout << "  I0     = " << f->GetParameter(0) << " +/- " << f->GetParError(0) << " mA\n"
                      << "  eta*Vt = " << f->GetParameter(1) << " +/- " << f->GetParError(1) << " mV\n"
                      << "  Chi2/NDF = " << f->GetChisquare() << "/" << f->GetNDF()
                      << " (Prob: " << f->GetProb() << ")\n";
        }
        else
        {
            std::cout << "  FIT FAILED\n";
        }
        std::cout << "-----------------------------------\n";
    };

    std::cout << std::fixed << std::setprecision(6);
    print_res("SILICON", f_si, r_si);
    print_res("GERMANIUM", f_ge, r_ge);
}