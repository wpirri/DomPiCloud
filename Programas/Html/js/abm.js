function getJsonHeaders(json_list) { 
	var headers = []; 
	for (var i = 0; i < json_list.length; i++) { 
		var row = json_list[i]; 
		for (var k in row) { 
			if ($.inArray(k, headers) == -1) { 
				headers.push(k); 
			} 
		} 
	} 
	return headers; 
}

function fillUserList(json_list, dst_div, index_label, edit_fcn, delete_fcn) { 
	// Getting the all column names 
	var headers = getJsonHeaders(json_list);
	var output = '';
	var i = 0;
	var j = 0;
	var index_value = '';

	// Header
	output += '<table class=abm-list-table>\n';
	output += '<tr>';
	for (i = 0; i < headers.length; i++) { 
		output += '<th>';
		output += headers[i];
		output += '</th>';
	}
	// Agrego las columnas de edición y borrado
	if(edit_fcn.length > 0)
		output += '<th>Editar</th>';
	output += '<th>Borrar</th>';
	output += '</tr>\n';
	// Datos
	for (i = 0; i < json_list.length; i++) {
		//if(json_list[i]['Id'] > 0) {
			output += '<tr>';
			index_value = '';
			for (j = 0; j < headers.length; j++) { 
				var val = json_list[i][headers[j]]; 
				// If there is any key, which is matching 
				// with the column name 
				if (val == null) val = "&nbsp;";   
				output += '<td>';
				output += val;
				output += '</td>';
				if(index_label == headers[j]) {
					index_value = val;
				}
			} 
			// Agrego los links de edición y borrado
			if(edit_fcn.length > 0) {
				val = '<td><img class="icon-link" src="../images/edit.png" OnClick="' + edit_fcn + '(\'' + index_value + '\');" ></td>' 
				output += val;
			}
			val = '<td><img class="icon-link" src="../images/delete.png" OnClick="' + delete_fcn + '(\'' + index_value + '\');" ></td>' 
			output += val;
			output += '</tr>\n';
		//}
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 

function fillUserNew(json_list, dst_div, save_fcn, cancel_fcn) {
	// Getting the all column names 
	var headers = getJsonHeaders(json_list);
	var output = '';
	var i = 0;

	// Header
	output += '<div class="abm-save-cancell"> <a class="menu-link" onclick="' + save_fcn + '();">Grabar</a>&nbsp;&nbsp;&nbsp;&nbsp;<a class="menu-link" onclick="' + cancel_fcn + '();">Cancelar</a>&nbsp;&nbsp;&nbsp;&nbsp;</div>\n';
	output += '<table class=abm-table id=abm_edit_table>\n';
	for (i = 0; i < headers.length; i++) { 
		output += '<tr>';
		output += '<th>';
		output += headers[i];
		output += '</th>';
		output += '<td>';
		output += '<input type="text" id="';
		output += headers[i] + '" name="';
		output += headers[i] + '" ';
		output += 'class="abm-edit-input-text" />';
		output += '</th>';
		output += '</tr>\n';
	}
	output += '</table>\n';
	output += '<br />\n';
	document.getElementById(dst_div).innerHTML = output;
}

function fillUserEdit(json_list, dst_div, save_fcn, cancel_fcn) { 
	// Getting the all column names 
	var headers = getJsonHeaders(json_list);
	var output = '';
	var i = 0;

	// Header
	output += '<div class="abm-save-cancell"> <a class="menu-link" onclick="' + save_fcn + '();">Grabar</a>&nbsp;&nbsp;&nbsp;&nbsp;<a class="menu-link" onclick="' + cancel_fcn + '();">Cancelar</a>&nbsp;&nbsp;&nbsp;&nbsp;</div>\n';
	output += '<table class=abm-table id=abm_edit_table>\n';
	for (i = 0; i < headers.length; i++) { 
		output += '<tr>';
		output += '<th>';
		output += headers[i];
		output += '</th>';
		var val = json_list[0][headers[i]]; 
		if (val == null || val == 'NULL') val = '';   
		output += '<td>';
		output += '<input type="text" id="';
		output += headers[i] + '" name="';
		output += headers[i] + '" ';
		output += 'class="abm-edit-input-text" value="';
		output += val;
		output += '" />';
		output += '</th>';
		output += '</tr>\n';
	}
	output += '</table>\n';
	output += '<br />\n';
	document.getElementById(dst_div).innerHTML = output;
} 

function fillUserDelete(json_list, dst_div, delete_fcn, cancel_fcn) {
	// Getting the all column names 
	var headers = getJsonHeaders(json_list);
	var output = '';
	var i = 0;

	// Header
	output += '<div class="abm-save-cancell"> <a class="menu-link" onclick="' + delete_fcn + '(\''+ json_list[0]['Usuario'] +'\');">Borrar</a>&nbsp;&nbsp;&nbsp;&nbsp;<a class="menu-link" onclick="' + cancel_fcn + '();">Cancelar</a>&nbsp;&nbsp;&nbsp;&nbsp;</div>\n';
	output += '<table class=abm-table id=abm_delete_table>\n';
	for (i = 0; i < headers.length; i++) { 
		output += '<tr>';
		output += '<th>';
		output += headers[i];
		output += '</th>';
		var val = json_list[0][headers[i]]; 
		if (val == null || val == 'NULL') val = '';   
		output += '<td>';
		output += val;
		output += '</th>';
		output += '</tr>\n';
	}
	output += '</table>\n';
	output += '<br />\n';
	document.getElementById(dst_div).innerHTML = output;
} 

function fillClientList(json_list, dst_div, index_label, status_fcn, delete_fcn) { 
	// Getting the all column names 
	var headers = getJsonHeaders(json_list);
	var output = '';
	var i = 0;
	var j = 0;
	var index_value = '';

	// Header
	output += '<table class=abm-list-table>\n';
	output += '<tr>';
	for (i = 0; i < headers.length; i++) { 
		output += '<th>';
		output += headers[i];
		output += '</th>';
	}
	// Agrego las columnas de edición y borrado
	if(status_fcn.length > 0)
		output += '<th>Estado</th>';
	output += '<th>Borrar</th>';
	output += '</tr>\n';
	// Datos
	for (i = 0; i < json_list.length; i++) {
		//if(json_list[i]['Id'] > 0) {
			output += '<tr>';
			index_value = '';
			for (j = 0; j < headers.length; j++) { 
				var val = json_list[i][headers[j]]; 
				// If there is any key, which is matching 
				// with the column name 
				if (val == null) val = "&nbsp;";   
				output += '<td>';
				output += val;
				output += '</td>';
				if(index_label == headers[j]) {
					index_value = val;
				}
			} 
			// Agrego los links de edición y borrado
			if(status_fcn.length > 0) {
				val = '<td><img class="icon-link" src="../images/edit.png" OnClick="' + status_fcn + '(\'' + index_value + '\');" ></td>' 
				output += val;
			}
			val = '<td><img class="icon-link" src="../images/delete.png" OnClick="' + delete_fcn + '(\'' + index_value + '\');" ></td>' 
			output += val;
			output += '</tr>\n';
		//}
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 

function fillClientStatus(json_list, dst_div, title, cancel_fcn) { 
	// Getting the all column names 
	var headers = getJsonHeaders(json_list);
	var output = '';
	var i = 0;
	var j = 0;
	var index_value = '';

	// Header
	output += '<h2>' + title + '</h2>';
	output += '<div class="abm-save-cancell"><a class="menu-link" onclick="' + cancel_fcn + '();">Cerrar</a>&nbsp;&nbsp;&nbsp;&nbsp;</div>\n';
	output += '<table class=abm-list-table>\n';
	output += '<tr>';
	for (i = 0; i < headers.length; i++) { 
		output += '<th>';
		output += headers[i];
		output += '</th>';
	}
	// Datos
	for (i = 0; i < json_list.length; i++) {
		//if(json_list[i]['Id'] > 0) {
			output += '<tr>';
			index_value = '';
			for (j = 0; j < headers.length; j++) { 
				var val = json_list[i][headers[j]]; 
				// If there is any key, which is matching 
				// with the column name 
				if (val == null) val = "&nbsp;";   
				output += '<td>';
				output += val;
				output += '</td>';
			} 
			output += '</tr>\n';
		//}
	}
	output += '</table>\n';
	output += '<br />\n';
	document.getElementById(dst_div).innerHTML = output;
} 
