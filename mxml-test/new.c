#include <stdio.h>
#include <mxml.h>

int main()
{
	mxml_node_t *new, *new1;
	mxml_index_t *ind;
	char str[800] = {0};

	new = mxmlNewXML("1.0");

	//mxmlAdd();

	new1 = mxmlNewElement(MXML_NO_PARENT, "SOAP-ENV:Envelope");

	new = mxmlNewText(new, 0, "SOAP-ENV:Envelope");

	//ind = mxmlIndexNew(new, "group", "type");
	//mxmlIndexReset(ind);

	//mxmlElementSetAttr(new1, "111", "222");

	mxmlAdd(new, MXML_ADD_AFTER, MXML_ADD_TO_PARENT, new1);

	mxmlSaveString(new, str, sizeof(str), MXML_NO_CALLBACK);

	printf("%s\n", str);

	//mxmlDelete(new);
	return 0;
}
