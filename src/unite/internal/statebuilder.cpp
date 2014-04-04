
#include "statebuilder.h"
#include "fqterm.h"
#include "fqterm_path.h"

#include <QDomDocument>
#include <QFile>

namespace FQTerm {

void StateBuilder::BuildState()
{
  QDomDocument domDocument;
  QFile welcome(getPath(RESOURCE) + "unite/welcom.xml");
  welcome.open(QIODevice::ReadOnly);
  QString errorStr;
  int errorLine;
  int errorColumn;
  domDocument.setContent(welcome.readAll(), true, &errorStr, &errorLine, &errorColumn);
  QDomElement root = domDocument.firstChildElement("page");
  if (root.isNull())
  {

  }
  QDomAttr a = root.attributeNode("type");
  //cout << a.value() << endl; 


}


}