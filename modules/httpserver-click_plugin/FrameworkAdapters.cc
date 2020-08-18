#include "FrameworkAdapters.hh"

int python2_check(){
  bool check_array[2] = {false, false};
  std::ifstream inFile("func.exe");
  for(std::string line; getline(inFile, line); ){
    if (line.find("def nf_main(token):") != std::string::npos) check_array[0] = true;
    if (line.find("def nf_stop(token):") != std::string::npos) check_array[1] = true;
  }
  inFile.close();
  if ((check_array[0]) && (check_array[1])) return 0;
  return 1;
}

void python2_adapter(){
  std::string killControl =  "\nimport socket\n\
import threading\n\
nf_token = [True]\n\
nf_thread = threading.Thread(target=nf_main, args=([nf_token]))\n\
nf_thread.start()\n\
wait_signal = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)\n\
wait_signal.bind(('', 8001))\n\
wait_signal.recvfrom(1024)\n\
wait_signal.close()\n\
nf_token[0] = False\n\
nf_stop(nf_token)\n\
nf_thread.join()";

  std::ofstream execFile;
  execFile.open("func.exe", std::ios_base::app);
  execFile << killControl;
  execFile.close();
}
