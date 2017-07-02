package net.irext.webapi;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;

import com.google.gson.Gson;
import net.irext.webapi.model.*;
import net.irext.webapi.utils.Constants;
import net.irext.webapi.request.*;
import net.irext.webapi.response.*;
import net.irext.webapi.utils.PackageUtils;
import net.irext.webapi.WebAPICallbacks.*;

import okhttp3.*;

import java.io.IOException;
import java.io.InputStream;

/**
 * Filename:       WebAPIs.java
 * Revised:        Date: 2017-03-30
 * Revision:       Revision: 1.0
 * <p>
 * Description:    HTTP Request initializer
 * <p>
 * Revision log:
 * 2017-03-30: created by strawmanbobi
 */
public class WebAPIs {

    @SuppressWarnings("all")
    private static final String TAG = WebAPIs.class.getSimpleName();

    private static WebAPIs mInstance = null;

    private static final String DEFAULT_ADDRESS = "http://irext.net";
    private static final String DEFAULT_APP = "/irext-server";
    private static String URL_PREFIX = DEFAULT_ADDRESS + DEFAULT_APP;

    private static final String SERVICE_SIGN_IN = "/app/app_login";
    private static final String SERVICE_LIST_CATEGORIES = "/indexing/list_categories";
    private static final String SERVICE_LIST_BRANDS = "/indexing/list_brands";
    private static final String SERVICE_LIST_PROVINCES = "/indexing/list_provinces";
    private static final String SERVICE_LIST_CITIES = "/indexing/list_cities";
    private static final String SERVICE_LIST_OPERATORS = "/indexing/list_operators";
    private static final String SERVICE_LIST_INDEXES = "/indexing/list_indexes";
    private static final String SERVICE_DOWNLOAD_BIN = "/operation/download_bin";
    private static final String SERVICE_ONLINE_DECODE = "/operation/decode";

    private int adminId;
    private String token;

    private OkHttpClient mHttpClient;

    private WebAPIs(String address, String appName) {
        if (null != address && null != appName) {
            URL_PREFIX = address + appName;
        }
        mHttpClient = new OkHttpClient();
    }

    private static void initializeInstance(String address, String appName) {
        mInstance = new WebAPIs(address, appName);
    }

    @SuppressWarnings("unused")
    public static WebAPIs getInstance(String address, String appName) {
        if (null == mInstance) {
            initializeInstance(address, appName);
        }
        return mInstance;
    }

    private String postToServer(String url, String json) throws IOException {
        MediaType JSON
                = MediaType.parse("application/json; charset=utf-8");

        RequestBody body = RequestBody.create(JSON, json);
        Request request = new Request.Builder()
                .url(url)
                .post(body)
                .build();
        Response response = mHttpClient.newCall(request).execute();
        return response.body().string();
    }

    private InputStream postToServerForOctets(String url, String json) throws IOException {
        MediaType JSON
                = MediaType.parse("application/json; charset=utf-8");
        RequestBody body = RequestBody.create(JSON, json);

        Request request = new Request.Builder()
                .url(url)
                .post(body)
                .build();

        Response response = mHttpClient.newCall(request).execute();
        return response.body().byteStream();
    }

    @SuppressWarnings("unused")
    public void signIn(Context context, SignInCallback signInCallback) {
        try {
            String signInURL = URL_PREFIX + SERVICE_SIGN_IN;
            AppSignInRequest appSignInRequest = new AppSignInRequest();

            ApplicationInfo appInfo = context.getPackageManager()
                    .getApplicationInfo(context.getPackageName(),
                            PackageManager.GET_META_DATA);
            String appKey = appInfo.metaData.getString("irext_app_key");
            String appSecret = appInfo.metaData.getString("irext_app_secret");

            appSignInRequest.setAppKey(appKey);
            appSignInRequest.setAppSecret(appSecret);
            appSignInRequest.setAppType(0);

            String packageName = context.getApplicationContext().getPackageName();
            appSignInRequest.setAndroidPackageName(packageName);

            String signature = PackageUtils.getCertificateSHA1Fingerprint(context);

            appSignInRequest.setAndroidSignature(signature);
            String bodyJson = appSignInRequest.toJson();

            String response = postToServer(signInURL, bodyJson);
            LoginResponse loginResponse = new Gson().fromJson(response, LoginResponse.class);
            if (loginResponse.getStatus().getCode() == Constants.ERROR_CODE_SUCCESS) {
                UserApp admin = loginResponse.getEntity();
                if (0 != admin.getId() && null != admin.getToken()) {
                    adminId = admin.getId();
                    token = admin.getToken();
                    signInCallback.onSignInSuccess(admin);
                } else {
                    signInCallback.onSignInFailed();
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
            signInCallback.onSignInError();
        }
    }

    @SuppressWarnings("unused")
    public void listCategories(int from, int count, ListCategoriesCallback listCategoriesCallback) {
        String listCategoriesURL = URL_PREFIX + SERVICE_LIST_CATEGORIES;
        ListCategoriesRequest listCategoriesRequest = new ListCategoriesRequest();
        listCategoriesRequest.setAdminId(adminId);
        listCategoriesRequest.setToken(token);
        listCategoriesRequest.setFrom(from);
        listCategoriesRequest.setCount(count);
        String bodyJson = listCategoriesRequest.toJson();

        try {
            String response = postToServer(listCategoriesURL, bodyJson);
            CategoriesResponse categoriesResponse = new Gson().fromJson(response, CategoriesResponse.class);

            if(categoriesResponse.getStatus().getCode() == Constants.ERROR_CODE_SUCCESS) {
                listCategoriesCallback.onListCategoriesSuccess(categoriesResponse.getEntity());
            } else {
                listCategoriesCallback.onListCategoriesFailed();
            }
        } catch (Exception e) {
            e.printStackTrace();
            listCategoriesCallback.onListCategoriesError();
        }
    }

    @SuppressWarnings("unused")
    public void listBrands(int categoryId, int from, int count,
                                  ListBrandsCallback listBrandsCallback) {
        String listBrandsURL = URL_PREFIX + SERVICE_LIST_BRANDS;
        ListBrandsRequest listBrandsRequest = new ListBrandsRequest();
        listBrandsRequest.setAdminId(adminId);
        listBrandsRequest.setToken(token);
        listBrandsRequest.setCategoryId(categoryId);
        listBrandsRequest.setFrom(from);
        listBrandsRequest.setCount(count);
        String bodyJson = listBrandsRequest.toJson();

        try {
            String response = postToServer(listBrandsURL, bodyJson);
            BrandsResponse brandsResponse = new Gson().fromJson(response, BrandsResponse.class);

            if (brandsResponse.getStatus().getCode() == Constants.ERROR_CODE_SUCCESS) {
                listBrandsCallback.onListBrandsSuccess(brandsResponse.getEntity());
            } else {
                listBrandsCallback.onListBrandsFailed();
            }
        } catch (Exception e) {
            e.printStackTrace();
            listBrandsCallback.onListBrandsError();
        }
    }

    @SuppressWarnings("unused")
    public void listProvinces(ListProvincesCallback listProvincesCallback) {
        String listProvincesURL = URL_PREFIX + SERVICE_LIST_PROVINCES;
        ListCitiesRequest listCitiesRequest = new ListCitiesRequest();
        listCitiesRequest.setAdminId(adminId);
        listCitiesRequest.setToken(token);
        String bodyJson = listCitiesRequest.toJson();

        try {
            String response = postToServer(listProvincesURL, bodyJson);
            CitiesResponse citiesResponse = new Gson().fromJson(response, CitiesResponse.class);

            if (citiesResponse.getStatus().getCode() == Constants.ERROR_CODE_SUCCESS) {
                listProvincesCallback.onListProvincesSuccess(citiesResponse.getEntity());
            } else {
                listProvincesCallback.onListProvincesFailed();
            }
        } catch (Exception e) {
            e.printStackTrace();
            listProvincesCallback.onListProvincesError();
        }
    }

    @SuppressWarnings("unused")
    public void listCities(String prefix, ListCitiesCallback listCitiesCallback) {
        String listCitiesURL = URL_PREFIX + SERVICE_LIST_CITIES;
        ListCitiesRequest listCitiesRequest = new ListCitiesRequest();
        listCitiesRequest.setAdminId(adminId);
        listCitiesRequest.setToken(token);
        listCitiesRequest.setProvincePrefix(prefix);
        String bodyJson = listCitiesRequest.toJson();

        try {
            String response = postToServer(listCitiesURL, bodyJson);
            CitiesResponse citiesResponse = new Gson().fromJson(response, CitiesResponse.class);

            if (citiesResponse.getStatus().getCode() == Constants.ERROR_CODE_SUCCESS) {
                listCitiesCallback.onListCitiesSuccess(citiesResponse.getEntity());
            } else {
                listCitiesCallback.onListCitiesFailed();
            }
        } catch (Exception e) {
            e.printStackTrace();
            listCitiesCallback.onListCitiesError();
        }
    }

    @SuppressWarnings("unused")
    public void listOperators(String cityCode,
                                           ListOperatersCallback listOperatersCallback) {
        String listOperatorsURL = URL_PREFIX + SERVICE_LIST_OPERATORS;
        ListOperatorsRequest listOperatorsRequest = new ListOperatorsRequest();
        listOperatorsRequest.setAdminId(adminId);
        listOperatorsRequest.setToken(token);
        listOperatorsRequest.setCityCode(cityCode);
        listOperatorsRequest.setFrom(0);
        listOperatorsRequest.setCount(20);
        String bodyJson = listOperatorsRequest.toJson();

        try {
            String response = postToServer(listOperatorsURL, bodyJson);
            OperatorsResponse operatorsResponse = new Gson().fromJson(response, OperatorsResponse.class);

            if (operatorsResponse.getStatus().getCode() == Constants.ERROR_CODE_SUCCESS) {
                listOperatersCallback.onListOperatorsSuccess(operatorsResponse.getEntity());
            } else {
                listOperatersCallback.onListOperatorsFailed();
            }
        } catch (Exception e) {
            e.printStackTrace();
            listOperatersCallback.onListOperatorsError();
        }
    }

    @SuppressWarnings("unused")
    public void listRemoteIndexes(int categoryId,
                                               int brandId,
                                               String cityCode,
                                               String operatorId,
                                               ListIndexesCallback onListIndexCallback) {
        String listIndexesURL = URL_PREFIX + SERVICE_LIST_INDEXES;
        ListIndexesRequest listIndexesRequest = new ListIndexesRequest();
        listIndexesRequest.setAdminId(adminId);
        listIndexesRequest.setToken(token);
        listIndexesRequest.setCategoryId(categoryId);
        listIndexesRequest.setBrandId(brandId);
        listIndexesRequest.setCityCode(cityCode);
        listIndexesRequest.setOperatorId(operatorId);
        listIndexesRequest.setFrom(0);
        listIndexesRequest.setCount(20);
        String bodyJson = listIndexesRequest.toJson();

        try {
            String response = postToServer(listIndexesURL, bodyJson);

            IndexesResponse indexesResponse = new Gson().fromJson(response, IndexesResponse.class);

            if (indexesResponse.getStatus().getCode() == Constants.ERROR_CODE_SUCCESS) {
                onListIndexCallback.onListIndexesSuccess(indexesResponse.getEntity());
            } else {
                onListIndexCallback.onListIndexesFailed();
            }
        } catch (Exception e) {
            e.printStackTrace();
            onListIndexCallback.onListIndexesError();
        }
    }

    @SuppressWarnings("unused")
    public void downloadBin(String remoteMap, int indexId,
                            DownloadBinCallback downloadBinCallback) {
        String downloadURL = URL_PREFIX + SERVICE_DOWNLOAD_BIN;
        DownloadBinaryRequest downloadBinaryRequest = new DownloadBinaryRequest();
        downloadBinaryRequest.setAdminId(adminId);
        downloadBinaryRequest.setToken(token);
        downloadBinaryRequest.setIndexId(indexId);

        String bodyJson = downloadBinaryRequest.toJson();

        if (null != bodyJson) {
            try {
                InputStream binStream = postToServerForOctets(downloadURL, bodyJson);

                if (null != binStream) {
                    downloadBinCallback.onDownloadBinSuccess(binStream);
                } else {
                    downloadBinCallback.onDownloadBinFailed();
                }
            } catch (IOException e) {
                e.printStackTrace();
                downloadBinCallback.onDownloadBinError();
            }
        }
    }

    @SuppressWarnings("unused")
    @Deprecated
    public int[] decodeIR(int indexId) {
        String decodeURL = URL_PREFIX + SERVICE_ONLINE_DECODE;
        DecodeRequest decodeRequest = new DecodeRequest();
        decodeRequest.setAdminId(adminId);
        decodeRequest.setToken(token);
        decodeRequest.setIndexId(indexId);

        String bodyJson = decodeRequest.toJson();

        if (null != bodyJson) {
            try {
                String response = postToServer(decodeURL, bodyJson);

                DecodeResponse decodeResponse = new Gson().fromJson(response, DecodeResponse.class);

                if (decodeResponse.getStatus().getCode() == Constants.ERROR_CODE_SUCCESS) {
                    return decodeResponse.getEntity();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return null;
    }
}
