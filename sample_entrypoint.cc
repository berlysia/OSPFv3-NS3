#include <iostream>
using namespace std;

void TestForOSPFHeader();
void TestForOSPFHello();
void TestForOSPFDatabaseDescription();
void TestForOSPFLinkStateRequest();
void TestForOSPFLinkStateUpdate();
void TestForOSPFLinkStateAck();

int main () {
    cout << "OSPF entrypoint - begin" << endl;
    TestForOSPFHeader();
    TestForOSPFHello();
    TestForOSPFDatabaseDescription();
    TestForOSPFLinkStateRequest();
    TestForOSPFLinkStateUpdate();
    TestForOSPFLinkStateAck();
    cout << "OSPF entrypoint - end" << endl;
}
