<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns="http://www.w3.org/1999/xhtml" name="index" version="1.0">
  <object:Response stage="{$gs_stage}" xml:id-policy-area="ignore">
    <gs:data>
      <interface:HTMLWebpage title="It Works!">
        <interface:HTMLContainer>
          <h1>It Works!</h1>
          <p>General Server is running.</p>
          <ul>
            <li><a href="/admin/webpage_response">Admin Suite</a></li>
          </ul>
        </interface:HTMLContainer>
      </interface:HTMLWebpage>
    </gs:data>
  </object:Response>

  <!-- Client-side fallback template (Firefox XSLT) -->
  <xsl:template match="object:Response">
    <html>
      <head><title>It Works!</title></head>
      <body style="font-family: sans-serif; max-width: 600px; margin: 4em auto; padding: 0 1em;">
        <h1>It Works!</h1>
        <p>General Server is running.</p>
        <ul>
          <li><a href="/admin/webpage_response">Admin Suite</a></li>
        </ul>
      </body>
    </html>
  </xsl:template>
</xsl:stylesheet>
