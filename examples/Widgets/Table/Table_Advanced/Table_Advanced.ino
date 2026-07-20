/*************************************************************
  Download latest Plynx library here:
    https://github.com/NickP005/plynx-library/releases/latest

  Plynx is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.plynx.cc
    Sketch generator:           http://examples.plynx.cc
    Plynx community:            http://community.plynx.cc
    Follow us:                  http://www.fb.com/plynxapp
                                http://twitter.com/plynx_app

  Plynx library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  Use Table widget to display simple value tables or events

  App project setup:
    Table widget on V1
    Button widget (push) on V10
    Button widget (push) on V11
 *************************************************************/

/* Comment this out to disable prints and save space */
#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <SPI.h>
#include <Ethernet.h>
#include <PlynxSimpleEthernet.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

WidgetTable table;
PLYNX_ATTACH_WIDGET(table, V1);

int rowIndex = 0;

// Button on V10 adds new items
PLYNX_WRITE(V10) {
  if (param.asInt()) {
    table.addRow(rowIndex, "Test row", millis() / 1000);
    table.pickRow(rowIndex);
    rowIndex++;
  }
}

// Button on V11 clears the table
PLYNX_WRITE(V11) {
  if (param.asInt()) {
    table.clear();
    rowIndex = 0;
  }
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  Plynx.begin(auth);

  // Setup table event callbacks
  table.onOrderChange([](int indexFrom, int indexTo) {
    Serial.print("Reordering: ");
    Serial.print(indexFrom);
    Serial.print(" => ");
    Serial.print(indexTo);
    Serial.println();
  });

  table.onSelectChange([](int index, bool selected) {
    Serial.print("Item ");
    Serial.print(index);
    Serial.print(selected ? " marked" : " unmarked");
  });
}

void loop()
{
  Plynx.run();
}

