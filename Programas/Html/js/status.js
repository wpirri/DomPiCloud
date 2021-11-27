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

function fillStatusTable(json_list, dst_div, title) { 
	// Getting the all column names 
	var headers = getJsonHeaders(json_list);
	var output = '<p class=status-table-title>&nbsp;' + title + '</p>\n<table class=status-list-table>\n';
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
		} 
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 

function fillStatusChangeTable(json_list, dst_div, title, index_label)
{ 
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
	output += '<th>&nbsp&nbsp&nbsp&nbsp</th>';
	output += '<th>&nbsp&nbsp&nbsp&nbsp</th>';
	output += '<th>&nbsp&nbsp&nbsp&nbsp</th>';
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
			if(index_label == headers[j]) {
				index_value = val;
			}
		} 
		// Agrego los links de edición y borrado
		val = '<td class=action-text onclick="control_object_on(\'' + index_value + '\');" >on</td>';
		output += val;
		val = '<td class=action-text onclick="control_object_off(\'' + index_value + '\');" >off</td>';
		output += val;
		val = '<td class=action-text onclick="control_object_switch(\'' + index_value + '\');" >switch</td>';
		output += val;
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 
