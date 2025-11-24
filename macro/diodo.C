#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TAxis.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TMath.h"
#include "TMatrixD.h"
#include <iostream>
#include <iomanip>

// Macro per analisi Diodi con fit esponenziali per Silicio e Germanio

void fit_calibrazione()
{
    const char *filename = "data/dati_calibrazione.txt";
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

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n>>> Calibrazione (" << filename << " ):\n";
    std::cout << "Intercetta: " << f0->GetParameter(0) << " +/- " << f0->GetParError(0) << std::endl;
    std::cout << "Pendenza  : " << f0->GetParameter(1) << " +/- " << f0->GetParError(1) << std::endl;
}

// Fit esponenziale: I(V) = I0 * exp(m * V)
void silicio()
{
    const char *filename = "data/dati_silicio.txt";
    TGraphErrors *gr = new TGraphErrors(filename, "%lg %lg %lg %lg");

    if (gr->GetN() == 0)
    {
        std::cout << "Errore: " << filename << " vuoto o non trovato!" << std::endl;
        return;
    }

    gStyle->SetOptFit(1111);
    gStyle->SetOptStat(0);
    gr->SetTitle("Diodo Silicio;Tensione (mV);Corrente (mA)");
    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(kBlue);

    // Modello di Shockley: I(V) = I0 * (exp(m*V) - 1)
    TF1 *f = new TF1("fshock_si", "[0]*(exp(x/[1])-1)", 0., 1000.);
    f->SetParNames("I0", "etaVt");
    // Valori iniziali ragionevoli per convergenza (eta*Vt ~ 26 mV a T ambiente)
    f->SetParameter(0, 1e-6);
    f->SetParameter(1, 60);
    f->SetLineColor(kRed);

    gr->Fit(f, "QS"); // Q=quiet, S=store result

    double I0 = f->GetParameter(0);
    double etaVt = f->GetParameter(1);
    double sigma_I0 = f->GetParError(0);
    double sigma_etaVt = f->GetParError(1);

    // Disegno (la funzione f disegnata rappresenta il fit eseguito)
    gr->Draw("AP");
    f->Draw("same");
    // Scala logaritmica sull'asse Y (corrente)
    if (gPad)
        gPad->SetLogy(1);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n>>> Silicio (" << filename << ") - fit Shockley (I=I0*(exp(m*V)-1)) :\n";
    std::cout << "I0 (saturazione)      = " << I0 << " +/- " << sigma_I0 << " (mA)" << std::endl;
    std::cout << "eta*Vt (parametro)    = " << etaVt << " +/- " << sigma_etaVt << " (mV)" << std::endl;
}

void germanio()
{
    const char *filename = "data/dati_germanio.txt";
    TGraphErrors *gr = new TGraphErrors(filename, "%lg %lg %lg %lg");

    if (gr->GetN() == 0)
    {
        std::cout << "Errore: " << filename << " vuoto o non trovato!" << std::endl;
        return;
    }
    gStyle->SetOptFit(1111);
    gStyle->SetOptStat(0);
    gr->SetTitle("Diodo Germanio;Tensione (mV);Corrente (mA)");
    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(kGreen + 2);

    TF1 *f = new TF1("fshock_ge", "[0]*(exp(x/[1])-1)", 0, 1000);
    f->SetParNames("I0", "etaVt");
    f->SetParameter(0, 0.005);
    f->SetParameter(1, 30);
    f->SetLineColor(kRed);

    gr->Fit(f, "QS");

    double I0 = f->GetParameter(0);
    double etaVt = f->GetParameter(1);
    double sigma_I0 = f->GetParError(0);
    double sigma_etaVt = f->GetParError(1);

    gr->Draw("AP");

    // Scala logaritmica sull'asse Y (corrente)
    if (gPad)
        gPad->SetLogy(1);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n>>> Germanio (" << filename << ") - fit Shockley (I=I0*(exp(m*V)-1)) :\n";
    std::cout << "I0 (saturazione)      = " << I0 << " +/- " << sigma_I0 << " (mA)" << std::endl;
    std::cout << "eta*Vt (parametro)    = " << etaVt << " +/- " << sigma_etaVt << " (mV)" << std::endl;
}

void analisi_completa()
{
    // Mantieni lo stile delle altre funzioni: crea i grafici direttamente dai file,
    // esegui i fit e poi usa TMultiGraph per sovrapporli in un unico pannello.
    gStyle->SetOptFit(1111);
    gStyle->SetOptStat(0);

    TCanvas *c1 = new TCanvas("c1", "Diodi - Multiplot fittato", 1000, 700);
    c1->cd();

    // Crea i TGraphErrors passando i nomi dei file (stile come nelle altre funzioni)
    const char *file_si = "data/dati_silicio.txt";
    const char *file_ge = "data/dati_germanio.txt";
    TGraphErrors *gr_si = new TGraphErrors(file_si, "%lg %lg %lg %lg");
    TGraphErrors *gr_ge = new TGraphErrors(file_ge, "%lg %lg %lg %lg");

    if (gr_si->GetN() == 0)
    {
        std::cout << "Errore: " << file_si << " vuoto o non trovato!" << std::endl;
        return;
    }
    if (gr_ge->GetN() == 0)
    {
        std::cout << "Errore: " << file_ge << " vuoto o non trovato!" << std::endl;
        return;
    }

    gr_si->SetMarkerStyle(20);
    gr_si->SetMarkerColor(kBlue);
    gr_si->SetTitle("Diodi - Multiplot;Tensione (mV);Corrente (mA)");
    gr_ge->SetMarkerStyle(21);
    gr_ge->SetMarkerColor(kRed);

    // Definisci le funzioni di fit (stesso modello usato altrove)
    TF1 *f_si = new TF1("f_si", "[0]*(exp(x/[1])-1)", 0., 1000.);
    f_si->SetParNames("I0", "etaVt");
    f_si->SetParameter(0, 1e-6);
    f_si->SetParameter(1, 40);
    f_si->SetLineColor(kBlue);
    TF1 *f_ge = new TF1("f_ge", "[0]*(exp(x/[1])-1)", 0., 110);
    f_ge->SetParNames("I0", "etaVt");
    f_ge->SetParameter(0, 0.005);
    f_ge->SetParameter(1, 30.0);
    f_ge->SetLineColor(kRed);

    // Esegui i fit direttamente sui grafici creati dai file
    TFitResultPtr r_si = gr_si->Fit(f_si, "QS");
    TFitResultPtr r_ge = gr_ge->Fit(f_ge, "QS", "", 0., 110.);

    // Assicuriamoci scala lineare
    if (gPad)
        gPad->SetLogy(1);

    // Multiplot con TMultiGraph
    TMultiGraph *mg = new TMultiGraph();
    mg->Add(gr_si, "P");
    mg->Add(gr_ge, "P");
    mg->SetTitle("Diodi: Silicio vs Germanio;Tensione (mV);Corrente (mA)");
    mg->Draw("AP");

    // Disegna i fit
    f_si->Draw("same");
    f_ge->Draw("same");

    // Legenda
    TLegend *leg = new TLegend(0.65, 0.65, 0.89, 0.89);
    leg->SetBorderSize(0);
    leg->AddEntry(gr_si, "Silicio (dati)", "p");
    leg->AddEntry(f_si, "Silicio (fit)", "l");
    leg->AddEntry(gr_ge, "Germanio (dati)", "p");
    leg->AddEntry(f_ge, "Germanio (fit)", "l");
    leg->Draw();

    // Stampa risultati
    std::cout << std::fixed << std::setprecision(6);
    if (r_si.Get())
    {
        std::cout << "\n>>> Silicio - fit results:\n";
        std::cout << "I0 = " << f_si->GetParameter(0) << " +/- " << f_si->GetParError(0) << " (mA)\n";
        std::cout << "eta*Vt = " << f_si->GetParameter(1) << " +/- " << f_si->GetParError(1) << " (mV)\n";
    }
    if (r_ge.Get())
    {
        std::cout << "\n>>> Germanio - fit results:\n";
        std::cout << "I0 = " << f_ge->GetParameter(0) << " +/- " << f_ge->GetParError(0) << " (mA)\n";
        std::cout << "eta*Vt = " << f_ge->GetParameter(1) << " +/- " << f_ge->GetParError(1) << " (mV)\n";
    }
}
