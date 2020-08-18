//VNF_HEADER
//VNF_VERSION: 1.0
//VNF_ID: 979eaf94-ad8c-4aef-90fa-c29189b42fde
//VNF_PROVIDER: UFSM
//VNF_NAME: Print
//VNF_RELEASE_DATE: 2017-04-08 21-45-45
//VNF_RELEASE_VERSION: 1.0
//VNF_RELEASE_LIFESPAN: 2017-06-08 21-45
//VNF_DESCRIPTION: Print received packets
//VNF_FRAMEWORK: Python2
//VNF_NETWORK: VirtIO
import time
import socket
import os

def nf_stop(token):
  s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  s.sendto("end_main", ("localhost", 8001))
  print("Sent: STOP!")
  return

def nf_main(token):
  s = socket.socket(socket.AF_INET, socket.SOCK_RAW)
  s.bind(("0.0.0.0", 0))

  while(token[0]):
    print("Wait: STOP!")
    d = s.recvfrom(65000)
    print("Received: STOP!")
