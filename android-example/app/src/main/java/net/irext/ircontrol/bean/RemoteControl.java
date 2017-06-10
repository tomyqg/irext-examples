package net.irext.ircontrol.bean;

import com.activeandroid.Model;
import com.activeandroid.annotation.Column;
import com.activeandroid.annotation.Table;
import com.activeandroid.query.Select;

import java.util.List;

/**
 * Filename:       RemoteControl.java
 * Revised:        Date: 2017-04-06
 * Revision:       Revision: 1.0
 * <p>
 * Description:    RemoteControl java bean
 * <p>
 * Revision log:
 * 2017-04-06: created by strawmanbobi
 */

@Table(name = "RemoteControl")
public class RemoteControl extends Model {

    @Column(name = "CategoryID")
    private int categoryId;

    @Column(name = "CategoryName")
    private String categoryName;

    @Column(name = "BrandID")
    private int brandId;

    @Column(name = "BrandName")
    private String brandName;

    @Column(name = "CityCode")
    private String cityCode;

    @Column(name = "CityName")
    private String cityName;

    @Column(name = "OperatorID")
    private String operatorId;

    @Column(name = "OperatorName")
    private String operatorName;

    @Column(name = "Remote")
    private String remote;

    @Column(name = "Protocol")
    private String protocol;

    @Column(name = "RemoteMap")
    private String remoteMap;

    @Column(name = "SubCategory")
    private int subCategory;

    public int getCategoryId() {
        return categoryId;
    }

    public void setCategoryId(int categoryId) {
        this.categoryId = categoryId;
    }

    public String getCategoryName() {
        return categoryName;
    }

    public void setCategoryName(String categoryName) {
        this.categoryName = categoryName;
    }

    public int getBrandId() {
        return brandId;
    }

    public void setBrandId(int brandId) {
        this.brandId = brandId;
    }

    public String getBrandName() {
        return brandName;
    }

    public void setBrandName(String brandName) {
        this.brandName = brandName;
    }

    public String getCityCode() {
        return cityCode;
    }

    public void setCityCode(String cityCode) {
        this.cityCode = cityCode;
    }

    public String getCityName() {
        return cityName;
    }

    public void setCityName(String cityName) {
        this.cityName = cityName;
    }

    public String getOperatorId() {
        return operatorId;
    }

    public void setOperatorId(String operatorId) {
        this.operatorId = operatorId;
    }

    public String getOperatorName() {
        return operatorName;
    }

    public void setOperatorName(String operatorName) {
        this.operatorName = operatorName;
    }

    public String getRemote() {
        return remote;
    }

    public void setRemote(String remote) {
        this.remote = remote;
    }

    public String getProtocol() {
        return protocol;
    }

    public void setProtocol(String protocol) {
        this.protocol = protocol;
    }

    public String getRemoteMap() {
        return remoteMap;
    }

    public void setRemoteMap(String remoteMap) {
        this.remoteMap = remoteMap;
    }

    public int getSubCategory() {
        return subCategory;
    }

    public void setSubCategory(int subCategory) {
        this.subCategory = subCategory;
    }

    public long getID() {
        return super.getId();
    }

    public RemoteControl(int categoryId, String categoryName, int brandId, String brandName,
                         String cityCode, String cityName, String operatorId, String operatorName,
                         String remote, String protocol, String remoteMap, int subCategory) {
        this.categoryId = categoryId;
        this.categoryName = categoryName;
        this.brandId = brandId;
        this.brandName = brandName;
        this.cityCode = cityCode;
        this.cityName = cityName;
        this.operatorId = operatorId;
        this.operatorName = operatorName;
        this.remote = remote;
        this.protocol = protocol;
        this.remoteMap = remoteMap;
        this.subCategory = subCategory;
    }

    public RemoteControl() {

    }

    public static List<RemoteControl> listRemoteControls(int from, int count) {
        return new Select()
                .from(RemoteControl.class)
                .orderBy("id DESC")
                .offset(from).limit(count)
                .execute();
    }

    public static long createRemoteControl(RemoteControl remoteControl) {
        return remoteControl.save();
    }

    public static RemoteControl getRemoteControl(long remoteID) {
        List<RemoteControl> remoteControls = new Select()
                .from(RemoteControl.class)
                .where("id = ?", remoteID)
                .execute();

        if (null != remoteControls && remoteControls.size() > 0) {
            return remoteControls.get(0);
        }
        return null;
    }
}
