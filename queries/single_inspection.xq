declare variable $inputDocument external;
declare function local:returnExpression ($v_vars as element(), $x as element())  {
	<expression>{
								if (data($v_vars/@compare_nom)="true") then (
											for $ev in $v_vars/value/ec
											return (
												if (count($ev/@id)) then (
													if (empty($x/../inspection[@nominal="true"]/var[@id=$ev/@id])) then (
														if (empty($x/../inspection[@nominal="true"]/var/var[@id=$ev/@id])) then (0)
														else string($x/../inspection[@nominal="true"]/var/var[@id=$ev/@id])
													) else (
														string($x/../inspection[@nominal="true"]/var[@id=$ev/@id])
													)
												) else (
													data($ev/@f)
												)
										),
										string("?")
									) else (),
									for $ev in $v_vars/value/ec
									return
										if (count($ev/@f)) then (
											data($ev/@f)
										) else if (count($ev/@sum)) then (
											for $sum in $x/../inspection[substring-before($x/@date, '.') = substring-before(@date, '.')]
											return ( if (empty($sum/var/var[@id=$ev/@sum])) then ()
													else string("+"), data($sum/var/var[@id=$ev/@sum]) )
										) else if (count($ev/@cc_attr)) then (
											for $att in $x/../@*
											return (
												if (name($att) = data($ev/@cc_attr)) then (
													data($att)
												) else ()
											)
										) else (
											if (empty($x/var[@id=$ev/@id])) then (
												if (empty($x/var/var[@id=$ev/@id])) then (0)
												else string($x/var/var[@id=$ev/@id])
											) else (
												string($x/var[@id=$ev/@id])
											)
										)
	}</expression>
};

declare function local:returnSubVar (  $i as element(), $v as element())  {

  let $iv := $i/var[@id=$v/@id]
  for $w in $v/var
		return (
		if (empty($iv/var[@id=$w/@id]) and not(count($w/value))) then ()
		else (
			let $iw := $iv/var[@id=$w/@id]
			return <tr move="true"><td style="text-align: right; width:50%;">{
						data($v/@name),
						<i18n>: </i18n>,
						if (empty(data($w/@name))) then (
							data($w/@id)
						) else ( data($w/@name) ),
						<i18n>: </i18n>
					}</td><td><table cellpadding="0" cellspacing="0"><tr><td align="right" valign="center"><var_value id="{ data($w/@id) }" date="{ data($i/@date) }">{
						if (empty($w/value)) then (
								if (data($w/@compare_nom)="true") then (
									<expression>{
									data($i/../inspection[@nominal="true"]/var[@id=$v/@id]/var[@id=$w/@id]),
									string("?"),
									data($iw)
									}</expression>
								)
								else data($iw)
							)
							else ( local:returnExpression ($w, $i) )
						}</var_value></td><td valign="center">{
							if (empty($w/@unit)) then ()
							else data($w/@unit)
						}</td></tr></table></td></tr>
				)
			)
 };

declare function local:returnVar ($i as element(), $v as element())  {

	let $iv := $i/var[@id=$v/@id]
	return if (empty($iv) and not(count($v/value))) then ()
	else (
					<tr move="true"><td style="text-align: right; width:50%;">{
						data($v/@name),
						<i18n>: </i18n>
						}</td><td><table cellpadding="0" cellspacing="0"><tr><td align="right" valign="center"><var_value id="{ data($v/@id) }" date="{ data($i/@date) }">{
							if (empty($v/value)) then (
								if (data($v/@compare_nom)="true") then (
									<expression>{
									data($i/../inspection[@nominal="true"]/var[@id=$v/@id]),
									string("?"),
									data($iv)
									}</expression>
								)
								else data($iv)
							)
							else ( local:returnExpression ($v, $i) )
						}</var_value></td><td valign="center">{
							if (empty($v/@unit)) then ()
							else data($v/@unit)
						}</td></tr></table></td></tr>
	)
};

declare function local:returnAnyVar ($i as element(), $v as element())  {

	if (count($v/var)) then (
			local:returnSubVar($i, $v)
		) else (
			local:returnVar($i, $v)
		)
};

declare function local:setWarnings ($warnings as element(), $circuit as element()) {
	<warnings>{
	<p align="center"><b><i18n>Error: Failed to process warnings.</i18n></b></p>,
	for $w in $warnings/warning
	return (
		<warning id="{data($w/@id)}">{
			for $c in $w/condition
			return (
				<condition>
				<first_expression>{
					if ($c/@cc_attr) then (
						<cc_attr>{
							for $att in $circuit/@*
							return (
								if (name($att) = data($c/@cc_attr)) then (
									<rem_dots>{data($att)}</rem_dots>
								) else ()
							)
						}</cc_attr>
					) else (
						if (count($c/value_ins)) then (
							for $ev in $c/value_ins/ec
							return (
								if (count($ev/@f)) then (
									data($ev/@f)
								) else if (count($ev/@cc_attr)) then (
									for $att in $circuit/@*
									return (
										if (name($att) = data($ev/@cc_attr)) then (
											data($att)
										) else ()
									)
								) else if (count($ev/@id)) then (
									<replace_id>{
										data($ev/@id)
									}</replace_id>
								) else ()
							)
						) else ()
					)
				}</first_expression>
				<f>{
					data($c/@f)
				}</f>
				<second_expression>{
					if (count($c/value_nom)) then (
						for $ev in $c/value_nom/ec
						return (
							if (count($ev/@f)) then (
								data($ev/@f)
							) else if (count($ev/@cc_attr)) then (
								for $att in $circuit/@*
								return (
									if (name($att) = data($ev/@cc_attr)) then (
										data($att)
									) else ()
								)
							) else if (count($ev/@id)) then (
								<replace_nom_id>{
									data($ev/@id)
								}</replace_nom_id>
							) else ()
						)
					) else (
						<rem_dots>{string($c)}</rem_dots>
					)
			}</second_expression>
			</condition>
			)
		}</warning>
	)
	}</warnings>
};

<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Inspection</title>
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
warning {
	font-size: xx-small;
	color: #FFFFFF;
}
-->
</style>
<script type="text/javascript" src="prototype.js"></script>
<script type="text/javascript" src="shared.js"></script>
<script type="text/javascript">
<!--
%1
-->
</script>
</head>
<body onLoad="onInspectionLoad(); translate();">

<table cellspacing="0" style="width:100%;">
{
let $d := doc($inputDocument)

let $circuit := $d/leaklog/customers/customer[@id="%2"]/circuit[@id="%3"]
let $i := $circuit/inspection[@date="%4"]
let $vars := $d/leaklog/variables
let $nom := $circuit/inspection[@nominal="true"]
return (
	<tr><td><table cellspacing="0" cellpadding="4" style="width:100%;">
	<tbody id="main_table_body">
	<tr style="background-color: #eee;"><td colspan="2" style="font-size: larger; width:100%; text-align: center;"><b><i18n>Circuit: </i18n>
	<a href="customer:{ data($i/../../@id) }/circuit:{ data($i/../@id) }">{data($i/../@id)}</a></b></td></tr>
	<tr style="background-color: #DFDFDF;"><td colspan="2" style="font-size: larger; width:100%; text-align: center;"><b>{
			if (data($i/@nominal)="true") then <i18n>Nominal inspection: </i18n>
			else <i18n>Inspection: </i18n>
		}<a href="customer:{ data($i/../../@id) }/circuit:{ data($i/../@id) }/inspection:{ data($i/@date) }/modify">
	{data($i/@date)}</a></b></td></tr>
	<tr><td><table id="table_l" cellspacing="0" cellpadding="4" style="width:100%;">{
	if (count($vars/var)) then (
	for $v in $vars/var
	return (
		local:returnAnyVar($i, $v)
	)
	) else ()
	}</table></td>
	<td style="width:50%;"><table id="table_r" cellspacing="0" cellpadding="4" style="width:100%;">
	</table></td></tr>
	</tbody>
	</table></td></tr>,
	<tr><td>
	<table cellspacing="0" cellpadding="4" style="width:100%;" id="warnings_element">
	<tr><td colspan="2" style="font-size: larger; width:100%;">
	<b><i18n>Warnings</i18n></b></td></tr>
	{
		local:setWarnings ($d/leaklog/warnings, $i/..)
	}
	</table></td></tr>,
	<table id="nominal_inspection" date="{ data($nom/@date) }">{
	if (exists($nom)) then (
		if (count($vars/var)) then (
			for $v in $vars/var
			return (
				local:returnAnyVar($nom, $v)
			)
		) else ()
	) else ()
	}</table>
)
}
</table>
</body>
</html>
