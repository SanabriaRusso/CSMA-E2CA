using namespace std;

double computeduration(double frame, int rate){
  int ndbps = rate;
  int plcp = 20;
  double bits = 16 + frame * 8 + 6;
  double symbols = ceil(bits / ndbps);
  int pause = 6;
  double duration = plcp + 4 * symbols	 + pause;

  return(duration);
}