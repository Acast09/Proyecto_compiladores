%top{

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/TokenLabel.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"

}

%{

extern CompilationStatus TokenLexemeAction(TokenLabel label);
extern CompilationStatus EOFLexemeAction();
extern CompilationStatus IgnoredLexemeAction();
extern CompilationStatus IPCidrLexemeAction();
extern CompilationStatus IPAddressLexemeAction();
extern CompilationStatus IdentifierLexemeAction();
extern CompilationStatus EnterMultilineCommentLexemeAction(FlexContext context);
extern CompilationStatus LeaveMultilineCommentLexemeAction();
extern CompilationStatus UnknownLexemeAction();

%}

%x MULTILINE_COMMENT

whitespace [[:space:]]+
ipv4_octet [[:digit:]]+
ipv4_address {ipv4_octet}"."{ipv4_octet}"."{ipv4_octet}"."{ipv4_octet}
ipv4_cidr {ipv4_address}"/"[[:digit:]]+
identifier [A-Za-z_][A-Za-z0-9_]*

%%

"topology"                              { return TokenLexemeAction(TOPOLOGY); }
"network"                               { return TokenLexemeAction(NETWORK); }
"router"                                { return TokenLexemeAction(ROUTER); }
"switch"                                { return TokenLexemeAction(SWITCH); }
"host"                                  { return TokenLexemeAction(HOST); }
"interface"                             { return TokenLexemeAction(INTERFACE); }
"connect"                               { return TokenLexemeAction(CONNECT); }
"route"                                 { return TokenLexemeAction(ROUTE); }
"via"                                   { return TokenLexemeAction(VIA); }

"/*"                                    { return EnterMultilineCommentLexemeAction(MULTILINE_COMMENT); }
<MULTILINE_COMMENT>"*/"                 { return LeaveMultilineCommentLexemeAction(); }
<MULTILINE_COMMENT>[^*]+                { return IgnoredLexemeAction(); }
<MULTILINE_COMMENT>.                    { return IgnoredLexemeAction(); }

{ipv4_cidr}                             { return IPCidrLexemeAction(); }
{ipv4_address}                           { return IPAddressLexemeAction(); }
{identifier}                             { return IdentifierLexemeAction(); }

"{"                                     { return TokenLexemeAction(OPEN_BRACE); }
"}"                                     { return TokenLexemeAction(CLOSE_BRACE); }
"."                                     { return TokenLexemeAction(DOT); }

{whitespace}                            { return IgnoredLexemeAction(); }
.                                       { return UnknownLexemeAction(); }

<<EOF>>                                 { return EOFLexemeAction(); }

%%

#include "FlexActions.h"
#include "FlexExport.h"