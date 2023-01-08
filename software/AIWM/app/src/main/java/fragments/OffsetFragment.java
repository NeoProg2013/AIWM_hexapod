package fragments;

import android.os.Bundle;

import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.application.aiwm.ControlActivityViewModel;
import com.application.aiwm.R;
import com.application.aiwm.joystick.CircleJoystick;


public class OffsetFragment extends Fragment {

    ControlActivityViewModel m_viewModel = null;

    public OffsetFragment() {
        // Required empty public constructor
    }
    @Override public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        Log.e("MotionFragment", "call onCreateView");
        View root = inflater.inflate(R.layout.fragment_offset, container, false);

        m_viewModel = new ViewModelProvider(getActivity()).get(ControlActivityViewModel.class);

        CircleJoystick joystick = root.findViewById(R.id.joystickOffset);
        joystick.setRangeX(-150, 150);
        joystick.setRangeY(-150, 150);
        joystick.m_x.observe(getViewLifecycleOwner(), v -> {
            ((TextView)root.findViewById(R.id.textViewOffsetX)).setText(v.toString());
            m_viewModel.swlp.setCurvature(v);
        });
        joystick.m_y.observe(getViewLifecycleOwner(), v -> {
            ((TextView)root.findViewById(R.id.textViewOffsetY)).setText(v.toString());
            m_viewModel.swlp.setDistance(v);
        });
        return root;
    }

    @Override public void onDestroy() {
        super.onDestroy();
        Log.e("MotionFragment", "call onDestroy");
    }

    @Override public void onDestroyView() {
        super.onDestroyView();
        Log.e("MotionFragment", "call onDestroyView");
    }
}