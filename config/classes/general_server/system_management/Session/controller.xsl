<xsl:stylesheet xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:conversation="http://general_server.org/xmlnamespaces/conversation/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="controller" controller="true" response:server-side-only="true" extension-element-prefixes="debug conversation database" version="1.0">
  <xsl:template match="object:Session" mode="login">
    <!-- use the Session::login.xsd xschema form to generate the HTML:form for logging in
      logout controller.xsl is on the User class
    -->
    <xsl:param name="username"/>
    <xsl:param name="password"/>

    <conversation:set-security-context username="{$username}" password="{$password}"/>

    <xsl:variable name="gs_user" select="conversation:user-node()"/>
    <xsl:if test="$gs_user">
      <database:hardlink-child select="$gs_user" destination="$gs_session" description="share user node in to the session during login"/>
      <debug:server-message output="login successful: {$username} / {$password}"/>
    </xsl:if>
    <xsl:else>
      <debug:server-message output="login failed (no user node): {$username} / {$password}"/>
    </xsl:else>
  </xsl:template>
</xsl:stylesheet>