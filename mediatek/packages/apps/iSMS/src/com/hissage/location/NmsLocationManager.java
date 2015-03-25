package com.hissage.location;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.params.HttpParams;
import org.apache.http.params.HttpProtocolParams;
import org.json.JSONArray;
import org.json.JSONObject;

import android.content.Context;
import android.location.Address;
import android.location.Geocoder;
import android.net.http.AndroidHttpClient;
import android.text.TextUtils;

import com.hissage.message.ip.NmsIpLocationMessage;
import com.hissage.util.log.NmsLog;

public class NmsLocationManager {

    private static String TAG = "NmsLocationManager";

    private static String PLACES_API_KEY = "AIzaSyApMEHePsNfd8M06zcx9lIXj8cRmYFzBgY"; // 9yinjian@gmail.com
    private static String PLACES_TYPES = "establishment";

    private static String URI_GEOCODE = "http://maps.google.com/maps/api/geocode/json?latlng="
            + "%1$f,%2$f&sensor=false&language=%3$s";
    private static String URI_PLACE = "https://maps.googleapis.com/maps/api/place/search/json?location="
            + "%1$f,%2$f&radius=%3$d&types="
            + PLACES_TYPES
            + "&sensor=false&language=%4$s&key="
            + PLACES_API_KEY;
    final static String URL_MAP = "http://maps.google.com/maps?q=loc:" + "%1$f,%2$f";

    // Please thread calls this function
    public static NmsLocation getAddress(double latitude, double longitude) {
        AndroidHttpClient client = AndroidHttpClient.newInstance("Android-Mms/2.0");
        HttpParams params = client.getParams();
        HttpProtocolParams.setContentCharset(params, "UTF-8");

        String uri = String.format(Locale.ENGLISH, URI_GEOCODE, latitude, longitude, getLanguageCode());

        NmsLocation loc = new NmsLocation(latitude, longitude);

        try {
            HttpGet httpGet = new HttpGet(uri);
            HttpResponse response = client.execute(httpGet);
            HttpEntity entity = response.getEntity();
            BufferedReader br = new BufferedReader(new InputStreamReader(entity.getContent()));
            StringBuffer sb = new StringBuffer();
            String result = null;
            while ((result = br.readLine()) != null) {
                sb.append(result);
            }
            if (sb.length() <= 1) {
                return null;
            }
            JSONObject jsonObject = new JSONObject(sb.toString());
            String status = jsonObject.getString("status");
            if (TextUtils.isEmpty(status) || !status.equalsIgnoreCase("OK")) {
                NmsLog.error(TAG, "getAddress: status=" + status + ", uri=" + uri);
                loc.setOk(false);
                client.close();
                return loc;
            } else {
                loc.setOk(true);
            }

            String vicinity = jsonObject.getJSONArray("results").getJSONObject(0)
                    .getString("formatted_address");
            loc.setVicinity(vicinity);

            String name = null;
            String sublocality = null;
            String locality = null;
            String adminAreaLevel1 = null;
            String country = null;
            JSONArray addrComponents = jsonObject.getJSONArray("results").getJSONObject(0)
                    .getJSONArray("address_components");
            for (int i = 0; i < addrComponents.length(); ++i) {
                JSONArray types = addrComponents.getJSONObject(i).getJSONArray("types");
                for (int j = 0; j < types.length(); ++j) {
                    String type = types.getString(j);
                    String shortName = addrComponents.getJSONObject(i).getString("short_name");
                    if (type.equals("sublocality")) {
                        sublocality = shortName;
                        break;
                    } else if (type.equals("locality")) {
                        locality = shortName;
                        break;
                    } else if (type.equals("administrative_area_level_1")) {
                        adminAreaLevel1 = shortName;
                        break;
                    } else if (type.equals("country")) {
                        country = shortName;
                        break;
                    } else {
                        NmsLog.trace(TAG, "type: " + type + ". ignore");
                    }
                }
            }

            if (!TextUtils.isEmpty(sublocality)) {
                name = sublocality;
            } else if (!TextUtils.isEmpty(locality)) {
                name = locality;
            } else if (!TextUtils.isEmpty(adminAreaLevel1)) {
                name = adminAreaLevel1;
            } else if (!TextUtils.isEmpty(country)) {
                name = country;
            } else {
                NmsLog.trace(TAG, "there is not suitable results. name == null");
            }

            if (!TextUtils.isEmpty(name)) {
                name = name.trim();
                loc.setName(name);
            }
        } catch (Exception e) {
            NmsLog.error(TAG, "getAddress: " + NmsLog.nmsGetStactTrace(e));
        } finally {
            client.close();
        }

        return loc;
    }

    // Please thread calls this function
    public static List<NmsLocation> getLocation(double latitude, double longitude, int radius) {
        AndroidHttpClient client = AndroidHttpClient.newInstance("Android-Mms/2.0");
        HttpParams params = client.getParams();
        HttpProtocolParams.setContentCharset(params, "UTF-8");

        String uri = String.format(Locale.ENGLISH ,URI_PLACE, latitude, longitude, radius, getLanguageCode());

        List<NmsLocation> locList = new ArrayList<NmsLocation>();

        try {
            HttpGet httpGet = new HttpGet(uri);
            HttpResponse response = client.execute(httpGet);
            HttpEntity entity = response.getEntity();
            BufferedReader br = new BufferedReader(new InputStreamReader(entity.getContent()));
            StringBuffer sb = new StringBuffer();
            String result = null;
            while ((result = br.readLine()) != null) {
                sb.append(result);
            }
            if (sb.length() <= 1) {
                return null;
            }
            JSONObject jsonObject = new JSONObject(sb.toString());
            String status = jsonObject.getString("status");
            if (TextUtils.isEmpty(status) || !status.equalsIgnoreCase("OK")) {
                NmsLog.error(TAG, "getLocation: status=" + status);
                client.close();
                return null;
            }

            JSONArray results = jsonObject.getJSONArray("results");
            for (int i = 0; i < results.length(); ++i) {
                boolean isBestMatch = false;
                JSONArray types = results.getJSONObject(i).getJSONArray("types");
                for (int j = 0; j < types.length(); ++j) {
                    String type = types.getString(j);
                    if (type.equals(PLACES_TYPES)) {
                        isBestMatch = true;
                        break;
                    }
                }
                if (isBestMatch) {
                    JSONObject geometry = results.getJSONObject(i).getJSONObject("geometry");
                    JSONObject location = geometry.getJSONObject("location");
                    double lat = location.getDouble("lat");
                    double lng = location.getDouble("lng");
                    String name = results.getJSONObject(i).getString("name");
                    String vicinity = results.getJSONObject(i).getString("vicinity");

                    NmsLocation loc = new NmsLocation(lat, lng, name, vicinity);
                    locList.add(loc);
                }
            }
        } catch (Exception e) {
            NmsLog.error(TAG, "getLocation: " + NmsLog.nmsGetStactTrace(e));
        } finally {
            client.close();
        }

        return locList;
    }

    // Please thread calls this function
    public static NmsLocation getLatLng(Context context, String searchAddr) {
        if (TextUtils.isEmpty(searchAddr)) {
            NmsLog.error(TAG, "searchAddr is invalid!");
            return null;
        }

        NmsLocation loc = new NmsLocation();

        Geocoder geocoder = new Geocoder(context, Locale.getDefault());
        try {
            List<Address> addresses = geocoder.getFromLocationName(searchAddr, 1);
            if (addresses != null && addresses.size() > 0) {
                Address address = addresses.get(0);
                double lat = address.getLatitude();
                double lng = address.getLongitude();
                // String name = address.getFeatureName();
                loc.setLatitude(lat);
                loc.setLongitude(lng);
                loc.setName(searchAddr);
            }
            return loc;
        } catch (Exception e) {
            NmsLog.error(TAG, "getLatLng: " + NmsLog.nmsGetStactTrace(e));
            return null;
        }
    }

    public static String getUrl(NmsIpLocationMessage locMsg) {
        if (locMsg == null) {
            NmsLog.error(TAG, "locMsg is null");
            return null;
        }

        String url = String.format(Locale.ENGLISH, URL_MAP, locMsg.latitude, locMsg.longitude);

        return url;
    }

    public static double formatLatLng(double latLng) {
        double ret = (double) (Math.round(latLng * 1E6)) / 1E6;

        return ret;
    }

    public static double formatLatLng(int e6LatLng) {
        double ret = (double) e6LatLng / 1E6;

        ret = formatLatLng(ret);

        return ret;
    }

    private static String getLanguageCode() {
        String languageCode = Locale.getDefault().getLanguage();
        return languageCode;
    }
}
