<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet 
    xmlns:oai="http://www.openarchives.org/OAI/2.0/"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
    xmlns:dc="http://purl.org/dc/elements/1.1/" 
    xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/" 
    xmlns:xlink="http://www.w3.org/1999/xlink"
    xmlns="http://www.loc.gov/mods/v3"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
    exclude-result-prefixes="oai_dc dc" 
    version="1.0">
    
    <!--
        A simplification of the LoC's DC to MODS 3.5 conversion script, adhering to 
        LDL MODS default guidance and informed by CDM to Islandora migration mappings.  
                
        This stylesheet will transform simple Dublin Core (DC) expressed in OAI DC 
        to MODS version 3.5. It will add LDL schema fields whose contents are updated 
        using parameters.
        
        [1] http://www.openarchives.org/OAI/openarchivesprotocol.html#MetadataNamespaces
        
    -->
    <xsl:output method="xml" indent="yes" encoding="UTF-8"/>
    
    <!-- Set parameters for source repository -->
    <xsl:param name="contributingRepository">
        <xsl:text>East Baton Rouge Parish Library</xsl:text>
    </xsl:param>
    <xsl:param name="institutionCode">
        <xsl:text>ebrpl</xsl:text>
    </xsl:param>
    <xsl:param name="contactInfo">
        <xsl:text>For more information about this item, contact batonrougedigitallibrary@ebrpl.com</xsl:text>
    </xsl:param>
    <xsl:param name="harvestedFrom">
        <xsl:text>East Baton Rouge Parish Library's Baton Rouge Archive</xsl:text> 
    </xsl:param>
    
    <!-- Set up transform and remove empty fields -->
    <xsl:template match="*[local-name()='OAI-PMH']">
        <xsl:apply-templates select="//oai_dc:dc"/>
    </xsl:template>
    <xsl:template match="@* | node() ">
        <xsl:copy>
            <xsl:apply-templates select="@*[normalize-space()] | node()[normalize-space()] "/>
        </xsl:copy>
    </xsl:template>
    
    <!-- Main template; orders resulting document fields and fills in elements with default and parameter values -->
    <xsl:template match="oai_dc:dc">
        <mods 
            xmlns="http://www.loc.gov/mods/v3"
            xmlns:mods="http://www.loc.gov/mods/v3"
            xmlns:xs="http://www.w3.org/2001/XMLSchema"
            xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xsi:schemaLocation="http://www.loc.gov/mods/v3 http://www.loc.gov/standards/mods/v3/mods-3-5.xsd"
            version="3.5" 
            >
        <xsl:apply-templates select="dc:title"/>
        <xsl:apply-templates select="dc:identifier"/>
        <xsl:apply-templates select="dc:creator"/>
        <xsl:apply-templates select="dc:contributor"/>
        <originInfo>
            <xsl:apply-templates select="dc:publisher"/>
            <xsl:apply-templates select="dc:date"/>
        </originInfo>
        <xsl:apply-templates select="dc:subject | dc:coverage"/>
        <xsl:apply-templates select="dc:description"/>
        <xsl:apply-templates select="dc:type"/>
        <physicalDescription>
            <xsl:apply-templates select="dc:format"/>
            <digitalOrigin>reformatted digital</digitalOrigin>
        </physicalDescription>
        <xsl:apply-templates select="dc:language"/>
        <name type="corporate" displayLabel="Contributing Repository">
            <namePart>
                <xsl:value-of select="$contributingRepository"/>
            </namePart>
            <role>
                <roleTerm type="text" authority="marcrelator">Contributing Repository</roleTerm>
                <roleTerm type="code" authority="marcrelator">rps</roleTerm>
            </role>
        </name>
        <xsl:apply-templates select="dc:source | dc:relation"/>
        <xsl:call-template name="RelationURL"/>
        <xsl:apply-templates select="dc:rights"/>
        <accessCondition type="use and reproduction" displayLabel="Contact Information">
            <xsl:value-of select="$contactInfo"/>
        </accessCondition>
        <recordInfo>
            <languageOfCataloging>
                <languageTerm type="code" authority="iso639-2b">eng</languageTerm>
            </languageOfCataloging>
                <recordOrigin>Record harvested from <xsl:value-of select="$harvestedFrom"/> using
                    OAI-PMH. DC converted to MODS by the LDL Development Team using a custom XSLT
                    1.0 stylesheet adapted from the LoC's DC to MODS 3.5 XSLT.</recordOrigin>
        </recordInfo>
        </mods>
    </xsl:template>
    
    <!-- DC Title maps to MODS titleInfo/title -->
    <xsl:template match="dc:title">
        <titleInfo>
            <title>
                <xsl:apply-templates/>
            </title>
        </titleInfo>
    </xsl:template>
    
    <!-- DC Identifier maps to MODS identifier;
    an identifier beginning with 'http' gets a @type value of 'uri' and a @displayLabel value of 'Harvested From'; 
    assumes that the only URL identifier in the source will be the URL of the record itself. -->
    <xsl:template match="dc:identifier">
        <identifier>
            <xsl:attribute name="type">
                <xsl:choose>
                    <xsl:when test="starts-with(text(), 'http')">
                        <xsl:text>uri</xsl:text>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:text>local</xsl:text>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:attribute>
            <xsl:if test="starts-with(text(), 'http')">
                <xsl:attribute name="displayLabel">
                    <xsl:text>Harvested From</xsl:text>
                </xsl:attribute>
            </xsl:if>
            <xsl:apply-templates/>
        </identifier>
    </xsl:template>
    
    <!-- DC Creator becomes MODS name/namePart with 'Creator' role -->
    <xsl:template match="dc:creator">
        <name>
            <namePart>
                <xsl:apply-templates/>
            </namePart>
            <role>
                <roleTerm type="text" authority="marcrelator">
                    <xsl:text>Creator</xsl:text>
                </roleTerm>
                <roleTerm type="code" authority="marcrelator">
                    <xsl:text>cre</xsl:text>
                </roleTerm>
            </role>
        </name>
    </xsl:template>
    
    <!-- DC Contributor becomes MODS name/namePart with 'Contributor' role -->
    <xsl:template match="dc:contributor">
        <name>
            <namePart>
                <xsl:apply-templates/>
            </namePart>
            <role>
                <roleTerm type="text" authority="marcrelator">
                    <xsl:text>Contributor</xsl:text>
                </roleTerm>
                <roleTerm type="code" authority="marcrelator">
                    <xsl:text>ctb</xsl:text>
                </roleTerm>
            </role>
        </name>
    </xsl:template>
    
    <!-- DC Publisher becomes MODS originInfo/publisher -->
    <xsl:template match="dc:publisher">
        <publisher>
            <xsl:apply-templates/>
        </publisher>
    </xsl:template>
    
    <!-- DC Date becomes MODS originInfo/dateOther;
    unable to assume all DC Dates will be Date Created -->
    <xsl:template match="dc:date">
        <dateOther>
            <xsl:apply-templates/>
        </dateOther>
    </xsl:template>
    
    <!-- DC Subject becomes MODS subject/topic; 
        calls template to split subjects on double hyphen -->
    <xsl:template match="dc:subject">
        <subject>
            <xsl:call-template name="tokenizeString">
                <xsl:with-param name="list" select="."/>
                <xsl:with-param name="delimiter" select="'--'"/>
                <xsl:with-param name="subelement" select="'topic'"/>
            </xsl:call-template>
        </subject>
    </xsl:template>
    
    <!-- DC Coverage becomes MODS subject/temporal if the contents of the field begin with a number 1-9 or hyphen;
        otherwise becomes MODS subject/geographic;
        calls template to split subjects on double hyphen -->
    <xsl:template match="dc:coverage">
        <subject>
            <xsl:call-template name="tokenizeString">
                <xsl:with-param name="list" select="."/>
                <xsl:with-param name="delimiter" select="'--'"/>
                <xsl:with-param name="subelement">
                    <xsl:choose>
                        <xsl:when test="string-length(text()) >= 3 and contains('0123456789-', substring(., 1, 1))">
                                <xsl:text>temporal</xsl:text>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:text>geographic</xsl:text>
                            </xsl:otherwise>
                    </xsl:choose>
                </xsl:with-param>
            </xsl:call-template>
        </subject>
    </xsl:template>
    
    <!-- DC Description becomes MODS abstract;
    assumes the majority of Descriptions will be summaries instead of notes;
    abstract field offers greater search and display functionality in LDL -->
    <xsl:template match="dc:description">
        <abstract>
            <xsl:apply-templates/>
        </abstract>
    </xsl:template>
    
    <!-- DC Type becomes MODS typeOfResource;
        DC Type Vocabulary values are translated to corresponding MODS Type Vocabulary;
        if there is no match, then 'mixed material' is assigned -->
    <xsl:template match="dc:type">
        <xsl:variable name="collection">
            <xsl:if test="../dc:type[string(text()) = 'collection' or string(text()) = 'Collection']">true</xsl:if>
        </xsl:variable>
        <xsl:variable name="smallcase" select="'abcdefghijklmnopqrstuvwxyz'" />
        <xsl:variable name="uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'" />
        <xsl:variable name="lowercasevalue" select="translate(string(text()), $uppercase, $smallcase)"/>
        <xsl:variable name="modsVocab">
            <xsl:choose>
                <xsl:when test="$lowercasevalue = 'image' or 'stillimage' or 'still image'">
                    <xsl:text>still image</xsl:text>
                </xsl:when>
                <xsl:when test="$lowercasevalue = 'sound'">
                    <xsl:text>sound recording</xsl:text>
                </xsl:when>
                <xsl:when test="$lowercasevalue = 'movingimage' or 'moving image'">
                    <xsl:text>moving image</xsl:text>
                </xsl:when>
                <xsl:when test="$lowercasevalue = 'physicalobject' or 'physical object'">
                    <xsl:text>three dimensional object</xsl:text>
                </xsl:when>
                <xsl:when test="$lowercasevalue = 'interactiveresource' or 'interactive resource' or 'service' or 'software'">
                    <xsl:text>software, multimedia</xsl:text>
                </xsl:when>
                <xsl:when test="$lowercasevalue = 'text'">
                    <xsl:text>text</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>mixed material</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <typeOfResource>
            <xsl:if test="$collection = 'true'">
                <xsl:attribute name="collection">
                    <xsl:text>yes</xsl:text>
                </xsl:attribute>
            </xsl:if>
            <xsl:value-of select="$modsVocab"/>
        </typeOfResource>                
    </xsl:template>   

    <!-- DC Format maps to MODS physicalDescription/*:
        - if value matches one of the specified file types, then maps to internetMediaType;
        - if value begins wtih a number 1-9, then maps to extent;
        - otherwise maps to note with @type='medium' -->
    <xsl:template match="dc:format">
        <xsl:choose>
            <xsl:when
                test="contains(., 'jpeg') or contains(., 'jpg') or contains(., 'tiff') or contains(., 'pdf') or contains(., 'mp3') or contains(., 'mp4')">
                <internetMediaType>
                    <xsl:apply-templates/>
                </internetMediaType>
            </xsl:when>
            <xsl:when test="contains('0123456789', substring(., 1, 1))">
                <extent>
                    <xsl:apply-templates/>
                </extent>
            </xsl:when>
            <xsl:otherwise>
                <note type="medium">
                    <xsl:apply-templates/>
                </note>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
    <!-- DC Language maps to MODS language;
    uses @type 'code' if the value is 2 or 3 characters long; 
    otherwise uses @type 'text' -->
    <xsl:template match="dc:language">
        <language>
            <xsl:choose>
                <xsl:when test="string-length(text()) = 3 or string-length(text()) = 2">
                    <languageTerm type="code">
                        <xsl:apply-templates/>
                    </languageTerm>
                </xsl:when>
                <xsl:otherwise>
                    <languageTerm type="text">
                        <xsl:apply-templates/>
                    </languageTerm>
                </xsl:otherwise>
            </xsl:choose>
        </language>
    </xsl:template>
    
    <!-- DC Source and DC Relation both map to MODS relatedItem with @type original:
        - if the value starts with http then maps to both location/url and identifier with @type 'uri';
        - otherwise maps to note with @type 'ownership' and @displayLabel 'Source Note' -->
    <xsl:template match="dc:source|dc:relation">
        <relatedItem type="original">
            <xsl:choose>
                <xsl:when test="starts-with(normalize-space(.),'http')">
                    <location>
                        <url>
                            <xsl:apply-templates/>
                        </url>
                    </location>
                    <identifier type="uri">
                        <xsl:apply-templates/>
                    </identifier>
                </xsl:when>
                <xsl:otherwise>
                    <note type="ownership" displayLabel="Source Note">
                        <xsl:apply-templates/>
                    </note>
                </xsl:otherwise>
            </xsl:choose>
        </relatedItem>
    </xsl:template>
    
    <!-- DC Rights maps to MODS accessCondition with @displayLabel 'Rights' -->
    <xsl:template match="dc:rights">
        <accessCondition type="use and reproduction" displayLabel="Rights">
            <xsl:apply-templates/>
        </accessCondition>
    </xsl:template>
    
    <!-- Relation URL at MODS relatedItem/location/url is built using LDL base URL, 
        an institution code passed as a parameter, 
        and collection code from the sibling setSpec -->
    <xsl:template name="RelationURL">
        <xsl:variable name="sourceURL" select="dc:identifier[starts-with(text(), 'http')]"/>
        <xsl:variable name="collectionCode">
            <xsl:value-of select="substring-before(substring-after($sourceURL, 'ref/collection/'), '/id/')"/>
        </xsl:variable>
        <xsl:variable name="relationURL">
            <xsl:value-of select="concat('http://louisianadigitallibrary.org/islandora/object/', $institutionCode, '-', $collectionCode, ':collection')"/>
        </xsl:variable>
        <relatedItem type="host">
            <location>
                <url displayLabel="Relation">
                    <xsl:value-of select="$relationURL"/>
                </url>
            </location>
        </relatedItem>
    </xsl:template>
    
    <!-- TokenizeString template used for splitting subject and coverage values on a delimiter -->
    <xsl:template name="tokenizeString">
        <xsl:param name="list"/>
        <xsl:param name="delimiter"/>
        <xsl:param name="subelement"/>
        <xsl:choose>
            <xsl:when test="contains($list, $delimiter)">
                <xsl:element name="{$subelement}">
                    <xsl:value-of select="substring-before($list,$delimiter)"/>
                </xsl:element>
                <xsl:call-template name="tokenizeString">
                    <xsl:with-param name="delimiter" select="$delimiter"/>
                    <xsl:with-param name="subelement" select="$subelement"/>
                    <xsl:with-param name="list" select="substring-after($list, $delimiter)"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:choose>
                    <xsl:when test="$list=''"/>
                    <xsl:otherwise>
                        <xsl:element name="{$subelement}">
                            <xsl:value-of select="$list"/>
                        </xsl:element>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
</xsl:stylesheet>
