package com.hissage.location;

import java.io.Serializable;

import android.graphics.Bitmap;

import com.hissage.util.log.NmsLog;

public class NmsLocation implements Cloneable, Serializable {

    private static final long serialVersionUID = 1L;

    private double longitude;
    private double latitude;
    private String name;
    private String vicinity;
    private boolean isSelected;
    private Bitmap mapImage;
    private String imagePath;
    private boolean isOk;

    public NmsLocation() {
    }

    public NmsLocation(double latitude, double longitude) {
        this.latitude = latitude;
        this.longitude = longitude;
    }

    public NmsLocation(double latitude, double longitude, String name, String vicinity) {
        this.latitude = latitude;
        this.longitude = longitude;
        this.name = name;
        this.vicinity = vicinity;
    }

    public void setLongitude(double longitude) {
        this.longitude = longitude;
    }

    public double getLongitude() {
        return longitude;
    }

    public void setLatitude(double latitude) {
        this.latitude = latitude;
    }

    public double getLatitude() {
        return latitude;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public void setVicinity(String vicinity) {
        this.vicinity = vicinity;
    }

    public String getVicinity() {
        return vicinity;
    }

    public void setSelected(boolean isSelected) {
        this.isSelected = isSelected;
    }

    public boolean isSelected() {
        return isSelected;
    }

    public void setMapImage(Bitmap mapImage) {
        this.mapImage = mapImage;
    }

    public Bitmap getMapImage() {
        return mapImage;
    }

    public void setImagePath(String imagePath) {
        this.imagePath = imagePath;
    }

    public String getImagePath() {
        return imagePath;
    }

    public void setOk(boolean isOk) {
        this.isOk = isOk;
    }

    public boolean isOk() {
        return isOk;
    }

    public NmsLocation clone() {
        NmsLocation ret = null;
        try {
            ret = (NmsLocation) super.clone();
        } catch (CloneNotSupportedException e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return ret;
    }
}
