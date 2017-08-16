#include <fcgiapp.h>
#include <string.h>

int main() {
	FCGX_Stream *in, *out, *err;
	FCGX_ParamArray envp;
	while (FCGX_Accept(&in, &out, &err, &envp) >= 0) {
		char *q = FCGX_GetParam("QUERY_STRING", envp);
		FCGX_FPrintF(out, "Content-type: text/plain\r\n\r\n");
		if (!q) {
			FCGX_FPrintF(out,
				"no query string: check web server configuration\n");
		}
		FCGX_FPrintF(out, "Query: '%s'\n", q);
		if (strcmp(q, "kancut") == 0) {
			FCGX_FPrintF(out, "Belel\n");		
		}
		else if (strcmp(q, "html") == 0) {
			FCGX_FPrintF(out, "<html>\n<head>\n<h2>HTML</h2>\n</html>\n");
		}
	}
	return 0;
}
