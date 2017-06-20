/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */ 

package Builders.swStateBuilder.access;

import java.util.Iterator;



public class Tabs {
	private int n = 0;
	private String tabs = "";
	public Tabs(){}
	public Tabs(int n){
		this.n = n;
		for (int i=0; i<n ;++i){
			tabs += "    "; // 4 spaces
		}
	}
	public String increase(int n){
		this.n += n;
		for (int i=0; i<n ;++i)
			tabs += "    "; // 4 spaces
		return tabs;
	}
	public String decrease(int n){
		if (this.n < n) return tabs;
		--this.n;
		for (int i=0; i<n ;++i)
			tabs = tabs.substring(4);
		return tabs;
	}
	public String get(){
		return tabs;
	}
}
