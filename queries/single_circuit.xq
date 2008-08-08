declare variable $inputDocument external;
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Circuit</title>
<style type="text/css">
<!--
body,td,th {
	font-family: "Lucida Grande", "Lucida Sans Unicode", verdana, lucida, sans-serif;
	font-size: small;
	color: #333333;
}
a:link {
	color: #333333;
	text-decoration: none;
}
a:visited {
	color: #333333;
	text-decoration: none;
}
a:hover {
	color: #660000;
	text-decoration: underline;
}
a:active {
	color: #333333;
	text-decoration: none;
}
-->
</style>
<script type="text/javascript" src="shared.js"></script>
<script type="text/javascript">
<!--
%1
-->
</script>
</head>
<body onLoad="translateTextValues(); translate();">

{
let $d := doc($inputDocument)
let $t := $d/leaklog/customers/customer[@id="%2"]/circuit[@id="%3"]

return (
	<table cellspacing="0" cellpadding="4" style="width:100%;">
	<tr style="background-color: #eee;"><td colspan="2" style="font-size: larger; width:100%; text-align: center;"><b><i18n>Company: </i18n>
	<a href="customer:{ data($t/../@id) }">{data($t/../@company)}</a></b></td></tr>
	<tr style="background-color: #DFDFDF;"><td colspan="2" style="font-size: large; width:100%; text-align: center;"><b><i18n>Circuit: </i18n>
	<a href="customer:{ data($t/../@id) }/circuit:{ data($t/@id) }/modify">
	{data($t/@id)}</a></b></td></tr>
	<tr><td><table cellspacing="0" cellpadding="4" style="width:100%;">
	<tr><td style="text-align: right; width:50%;"><b><i18n>Manufacturer: </i18n></b></td><td style="width:50%;"><b> {
		data($t/@manufacturer)
	}</b></td></tr>
	<tr><td style="text-align: right;"><i18n>Type: </i18n></td><td> {
		data($t/@type)
	}</td></tr>
	<tr><td style="text-align: right;"><i18n>Serial number: </i18n></td><td> {
		data($t/@sn)
	}</td></tr>
	<tr><td style="text-align: right; width:50%;"><i18n>Year of purchase: </i18n></td><td> {
		data($t/@year)
	}</td></tr>
	<tr><td style="text-align: right; width:50%;"><i18n>Date of commissioning: </i18n></td><td style="width:50%;"> {
		data($t/@commissioning)
	}</td></tr>
	<tr><td style="text-align: right; width:50%;"><i18n>Field of application: </i18n></td><td style="width:50%;"><textvalue type="field"> {
		data($t/@field)
	}</textvalue></td></tr>
	</table></td>
	<td><table cellspacing="0" cellpadding="4" style="width:100%;">
	<tr><td style="text-align: right; width:50%;"><i18n>Refrigerant: </i18n></td><td style="width:50%;"> {
		data($t/@refrigerant)
	}</td></tr>
	<tr><td style="text-align: right;"><i18n>Amount of refrigerant: </i18n></td><td> {
		data($t/@refrigerant_amount),
		string("kg")
	}</td></tr>
	<tr><td style="text-align: right;"><i18n>Oil: </i18n></td><td> {
		data($t/@oil)
	}</td></tr>
	<tr><td style="text-align: right;"><i18n>Amount of oil: </i18n></td><td> {
		data($t/@oil_amount),
		string("kg")
	}</td></tr>
	<tr><td style="text-align: right;"><i18n>Service life: </i18n></td><td> {
		data($t/@life),
		<i18n> years</i18n>
	}</td></tr>
	<tr><td style="text-align: right;"><i18n>Run-time per day: </i18n></td><td> {
		data($t/@runtime),
		<i18n> hours</i18n>
	}</td></tr>
	<tr><td style="text-align: right;"><i18n>Rate of utilisation: </i18n></td><td> {
		data($t/@utilisation),
		string("%")
	}</td></tr>
	</table></td></tr>
	</table>,
	
	<table cellspacing="0" cellpadding="4" style="width:100%;">
	<tr><td rowspan="{
				if (count($t/inspection) = 0) then 2
				else if (count($t/inspection) mod 2 = 0) then count($t/inspection) div 2 + 1
				else xs:integer(count($t/inspection) div 2 + 2)
			}" style="width:10%;"/>
	<td colspan="2" style="background-color: #eee; font-size: medium; text-align: center; width:80%;"><b>
	<a href="customer:{ data($t/../@id) }/circuit:{ data($t/@id) }/table">
	<i18n>Inspections</i18n></a></b></td>
	<td rowspan="{
				if (count($t/inspection) = 0) then 2
				else if (count($t/inspection) mod 2 = 0) then count($t/inspection) div 2 + 1
				else xs:integer(count($t/inspection) div 2 + 2)
			}" style="width:10%;"/></tr>
	<tr><td style="width:40%;"><table cellspacing="0" cellpadding="4" style="width:100%;">{
	for $i in $t/inspection[xs:integer(count($t/inspection) div 2 +0.5) >= position()]
	return (
		<tr><td style="text-align: center">{
			if (data($i/@nominal) = "true") then (
				<i18n>Nominal: </i18n>
			) else (),
			<a href="customer:{ data($t/../@id) }/circuit:{ data($t/@id) }/inspection:{ data($i/@date) }">{data($i/@date)}</a>
		}</td></tr>
	)
	}</table></td>
	<td style="width:40%;"><table cellspacing="0" cellpadding="4" style="width:100%;">{
	for $i in $t/inspection[xs:integer(count($t/inspection) div 2 +0.5) < position()]
	return (
		<tr><td style="text-align: center">{
			if (data($i/@nominal) = "true") then (
				<i18n>Nominal: </i18n>
			) else (),
			<a href="customer:{ data($t/../@id) }/circuit:{ data($t/@id) }/inspection:{ data($i/@date) }">{data($i/@date)}</a>
		}</td></tr>
	)
	}</table></td></tr>
	</table>
)
}

</body>
</html>
