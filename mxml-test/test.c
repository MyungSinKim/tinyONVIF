#include <stdio.h>
#include <string.h>
#include <mxml.h>

mxml_type_t type_cb(mxml_node_t *node)
{
	const char *type;

	printf("node->type = %d\n", node->type);

	printf("node->value.element.name = %s\n", node->value.element.name);
	printf("node->value.element.num_attrs = %d\n", node->value.element.num_attrs);
	
	printf("node->value.element.attrs->name = %s\n", node->value.element.attrs->name);
	printf("node->value.element.attrs->value = %s\n", node->value.element.attrs->value);

	printf("node->value.integer = %d\n", node->value.integer);

	printf("node->value.opaque = %s\n", node->value.opaque);


	//type = mxmlElementGetAttr(node, "GetCapabilities");

	//printf("type = %s\n", type);

	return 0;

#if 0
  const char	*type;			/* Type string */


 /*
  * You can lookup attributes and/or use the element name, hierarchy, etc...
  */


  if ((type = mxmlElementGetAttr(node, "type")) == NULL)
    type = node->value.element.name;

	printf("type = %s\n", type);

  if (!strcmp(type, "integer"))
    return (MXML_INTEGER);
  else if (!strcmp(type, "opaque") || !strcmp(type, "pre"))
    return (MXML_OPAQUE);
  else if (!strcmp(type, "real"))
    return (MXML_REAL);
  else
    return (MXML_TEXT);
#endif    
}

	
int parse_xml(char *xml_str)
{
	mxml_node_t *tree;
	tree = mxmlLoadString(NULL, xml_str, type_cb);

	return 0;
}

int main()
{
	FILE *fp;
	char buff[1024*2] = {0};
	
	// read xml
	if (NULL == (fp = fopen("getcapbilities.txt", "rb")))
	{
		perror("fopen");
		return -1;
	}

	if (0 >= fread(buff, 1, 1024*2, fp))
	{
		perror("fread");
		return -1;
	}

	//printf("%s\n", buff);
	parse_xml(buff);


	fclose(fp);
	return 0;
}
