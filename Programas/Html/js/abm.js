function constructTable(json_list, dst_tbl) { 
	// Getting the all column names 
	var cols = constructTableHeaders(json_list, dst_tbl);   
	// Traversing the JSON data 
	for (var i = 0; i < json_list.length; i++) { 
		var row = $('<tr/>');    
		for (var colIndex = 0; colIndex < cols.length; colIndex++) { 
			var val = json_list[i][cols[colIndex]]; 
			// If there is any key, which is matching 
			// with the column name 
			if (val == null) val = "&nbsp;";   
			row.append($('<td/>').html(val)); 
		} 
		// Adding each row to the table 
		$(dst_tbl).append(row); 
	} 
} 

function constructTableHeaders(json_list, dst_tbl) { 
	var columns = []; 
	var header = $('<tr/>'); 
	for (var i = 0; i < json_list.length; i++) { 
		var row = json_list[i]; 
		for (var k in row) { 
			if ($.inArray(k, columns) == -1) { 
				columns.push(k); 
				// Creating the header 
				header.append($('<th/>').html(k)); 
			} 
		} 
	} 
    // Appending the header to the table 
	$(dst_tbl).append(header); 
	return columns; 
}

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

function fillAbmList(json_list, dst_div, title, index_label, edit_link, delete_link) { 
	// Getting the all column names 
	var headers = getJsonHeaders(json_list);
	var output = '<p class=abm-table-title>&nbsp;' + title + '</p>\n<table class=abm-list-table>\n';
	var i = 0;
	var j = 0;
	var index_value = '';

	// Header
	output += '<tr>';
	for (i = 0; i < headers.length; i++) { 
		output += '<th>';
		output += headers[i];
		output += '</th>';
	}
	// Agrego las columnas de edición y borrado
	if(edit_link.length > 0)
		output += '<th>Editar</th>';
	output += '<th>Borrar</th>';
	output += '</tr>\n';
	// Datos
	for (i = 0; i < json_list.length; i++) {
		if(json_list[i]['Id'] > 0) {
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
			if(edit_link.length > 0) {
				val = '<td><a href="' + edit_link + '?' + index_label + '=' + index_value + '"><img src="../images/edit.png"></a></td>' 
				output += val;
			}
			val = '<td><a href="' + delete_link + '?' + index_label + '=' + index_value + '"><img src="../images/delete.png"></a></td>' 
			output += val;
			output += '</tr>\n';
		}
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 

function fillAbmDelete(json_list, dst_div, title) { 
	// Getting the all column names 
	var headers = getJsonHeaders(json_list);
	var output = '<p class=abm-table-title>&nbsp;' + title + '</p>\n<table class=abm-table id=abm_delete_table>\n';
	var i = 0;

	// Header
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
	document.getElementById(dst_div).innerHTML = output;
} 

function fillAbmEdit(json_list, dst_div, title) { 
	// Getting the all column names 
	var headers = getJsonHeaders(json_list);
	var output = '<p class=abm-table-title>&nbsp;' + title + '</p>\n<table class=abm-table id=abm_edit_table>\n';
	var i = 0;

	// Header
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
	document.getElementById(dst_div).innerHTML = output;
} 

function fillAbmForm(json_list, dst_div, title)
{
	// Getting the all column names 
	var headers = getJsonHeaders(json_list);
	var output = '<p class=abm-table-title>' + title + '</p>\n<table class=abm-table id=abm_edit_table>\n';
	var i = 0;

	// Header
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
	document.getElementById(dst_div).innerHTML = output;
}

function fillAddAssignList(json_list, dst_div, title, add_fcn) { 
	// Getting the all column names 
	var headers = getJsonHeaders(json_list);
	var output = '<p class=abm-table-title>&nbsp;' + title + '</p>\n<table class=abm-list-table>\n';
	var i = 0;
	var j = 0;
	var index_value = '';

	// Header
	output += '<tr>';
	for (i = 0; i < headers.length; i++) { 
		output += '<th>';
		output += headers[i];
		output += '</th>';
	}
	// Agrego las columnas de edición y borrado
	output += '<th>Agregar</th>';
	output += '</tr>\n';
	// Datos
	for (i = 0; i < json_list.length; i++) { 
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
			if(headers[j] == 'Id') {
				index_value = val;
			}
		} 
		// Agrego los links de edición y borrado
		val = '<td><img src="../images/edit.png" OnClick="' + add_fcn + '(' + index_value + ');"></a></td>' 
		output += val;
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 
