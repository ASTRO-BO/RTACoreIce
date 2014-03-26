void drawCamera() {
	TCanvas* c1 = new TCanvas;
	c1->Range(-10, -10, 10, 10);
	for(int i=0; i<30; i++) {
		for(int j=0; j<30; j++) {
			TBox* b = new TBox(0.0 + i*0.2, 0.0 + j * 0.2, 0.2 + i * 0.2, 0.2 + j * 0.2);
			b->Draw("AL");
		}
	}
}