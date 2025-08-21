/*****************************************
 * 算法/相机参数由XML文件组织，加载 XML 文件.
 * 文件组织:
 * <Parameters>
 *	<GroupBox "name" = title>
 *		<Parameter>
 *			<Info key = "name" label = "名称" value = "0" type = "int"/>
 *			<Fields values = "0,1,2,3,4",range = "0,5">
 *		</Parameter>
 *	</GroupBox>
 * </Parameters>
 *
 * 文件中记录了参数的 key-label-value 组合
 * 然后将参数显示在界面上需要与 QT
 *****************************************/

#pragma once
