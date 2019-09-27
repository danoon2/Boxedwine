package org.boxedwine;

import android.Manifest;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.res.AssetManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.support.design.widget.BottomNavigationView;
import android.support.v7.app.AppCompatActivity;
import android.support.annotation.NonNull;
import android.util.Log;
import android.view.MenuItem;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.List;
import java.util.Vector;

import pub.devrel.easypermissions.EasyPermissions;

public class BoxedWineUiActivity extends AppCompatActivity implements EasyPermissions.PermissionCallbacks {
    private TextView mTextMessage;
    private Vector<BoxedContainer> containers;

    private BottomNavigationView.OnNavigationItemSelectedListener mOnNavigationItemSelectedListener = new BottomNavigationView.OnNavigationItemSelectedListener() {

        @Override
        public boolean onNavigationItemSelected(@NonNull MenuItem item) {
            switch (item.getItemId()) {
                case R.id.navigation_apps:
                    mTextMessage.setText(R.string.title_apps);
                    return true;
                case R.id.navigation_containers:
                    mTextMessage.setText(R.string.title_containers);
                    return true;
                case R.id.navigation_install:
                    mTextMessage.setText(R.string.title_install);
                    return true;
            }
            return false;
        }
    };

    private void installFileSystem() {
        //check if app has permission to write to the external storage.
        boolean storagePermission = EasyPermissions.hasPermissions(this, Manifest.permission.WRITE_EXTERNAL_STORAGE);

        if (!storagePermission) {
            EasyPermissions.requestPermissions(this, getString(R.string.permission_storage), 0, Manifest.permission.WRITE_EXTERNAL_STORAGE);
        } else {
            extractWine();
            GlobalSettings.initWineVersions();
            onInstallFirstWineVersion();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        EasyPermissions.onRequestPermissionsResult(requestCode, permissions, grantResults, this);
    }

    @Override
    public void onPermissionsGranted(int requestCode, List<String> perms) {
        installFileSystem();
    }

    @Override
    public void onPermissionsDenied(int requestCode, List<String> perms) {
    }

    private void extractWine() {
        BoxedUtils.extraAsset(getAssets(), getExternalFilesDir("FileSystems"), "wine17.zip");
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_boxed_wine_ui);
        BottomNavigationView navView = findViewById(R.id.nav_view);
        mTextMessage = findViewById(R.id.message);
        navView.setOnNavigationItemSelectedListener(mOnNavigationItemSelectedListener);

        GlobalSettings.dataFolderLocation = this.getExternalFilesDir("").getAbsolutePath();
        GlobalSettings.initWineVersions();
        loadContainers();
        if (GlobalSettings.wineVersions.size()==0) {
            installFileSystem();
        } else {
            if (this.containers.size()>0) {
                BoxedContainer container = this.containers.elementAt(0);
                if (container.getAppCount()>0) {
                    final BoxedApp app = container.getApp(0);

                    final Handler handler = new Handler();
                    handler.postDelayed(new Runnable() {
                        @Override
                        public void run() {
                            app.launch(BoxedWineUiActivity.this);
                        }
                    }, 100);
                }
            }
        }
    }

    private void loadContainers() {
        File dir = new File(GlobalSettings.getContainerFolder());

        this.containers = new Vector<>();

        if (dir.exists() && dir.isDirectory()) {
            for (File f : dir.listFiles()) {
                if (f.isDirectory()) {
                    BoxedContainer container = new BoxedContainer();
                    if (container.load(f.getAbsolutePath())) {
                        this.containers.addElement(container);
                    }
                }
            }
        }
    }

    private void onInstallFirstWineVersion() {
        if (GlobalSettings.wineVersions.size()>0) {
            File containerFileDir = new File(GlobalSettings.getContainerFolder() + File.separator + "Default");
            containerFileDir.mkdirs();

            BoxedContainer container = BoxedContainer.createContainer(containerFileDir.getAbsolutePath(), "Default", GlobalSettings.wineVersions.elementAt(0).name);

            BoxedApp app = new BoxedApp("WineMine", "/home/username/.wine/drive_c/windows/system32", "winemine.exe", container);
            app.saveApp();

            // :TODO: unzip winemine.exe into that location so that icon works
            String targetPath = containerFileDir.getAbsolutePath() + File.separator + "root";
            String targetFile = "home" + File.separator + "username" + File.separator + ".wine" + File.separator + "drive_c" + File.separator + "windows" + File.separator + "system32" + File.separator + "winemine.exe";

            BoxedUtils.extractFileFromZip(new File(GlobalSettings.wineVersions.elementAt(0).filePath), targetFile, new File(targetPath + File.separator + targetFile));

            String drive_c = targetPath + File.separator + "home" + File.separator + "username" + File.separator + ".wine" + File.separator + "drive_c" + File.separator + "games";
            BoxedUtils.extraAsset(getAssets(), new File(drive_c), "PERF_W95.EXE");

            BoxedApp app1 = new BoxedApp("MDK Perf", "/home/username/.wine/drive_c/games", "c:\\games\\PERF_W95.EXE", container);
            app1.saveApp();

            loadContainers();
            app1.launch(this);
        }
    }
}
